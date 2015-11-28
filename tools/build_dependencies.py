#!/usr/bin/env python

from __future__ import print_function
import argparse
import copy
import glob
import logging
import mako.template
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
import tarfile
import yaml
from itertools import chain
try:
  from urllib.request import urlopen
except:
  from urllib2 import urlopen

import buildenv

logger = logging.getLogger(__name__)


def concurrency():
  try:
    return multiprocessing.cpu_count()
  except NotImplementedError:
    return 4


class BuildConf(object):
  def __init__(self, cc, cxx, shared, debug, jobs, installdir):
    self.env = copy.deepcopy(os.environ)
    self.installdir = installdir
    buildenv.setup_env(self.installdir, self.env)
    self.win = platform.system() == 'Windows'
    self.make = ['nmake'] if self.win else ['ninja', '-j', '%d' % jobs]
    self.cc = cc
    self.cxx = cxx
    self.shared = shared
    self.debug = debug
    if self.win:
      logger.info('Platform is Windows')
      self.libprefix = ''
      self.libext = '.dll' if shared else '.lib'
      self.execext = '.exe'
    else:
      self.libprefix = 'lib'
      self.libext = '.so' if shared else '.a'
      self.execext = ''

  @property
  def shared_on(self):
    return 'ON' if self.shared else 'OFF'

  @property
  def build_type(self):
    return 'Debug' if self.debug else 'Release'

  def cmake_cmd(self, extra_args, cmake_dir='.'):
    cmd = [
      'cmake',
      '-DBUILD_SHARED_LIBS=' + self.shared_on,
      '-DCMAKE_BUILD_TYPE=' + self.build_type,
    ] + extra_args
    generator = 'NMake Makefiles' if self.win else 'Ninja'
    cmd.append('-G' + generator)
    if self.cc:
      cmd.append('-DCMAKE_C_COMPILER=' + self.cc)
    if self.cxx:
      cmd.append('-DCMAKE_CXX_COMPILER=' + self.cxx)
    cmd.append(cmake_dir)
    return cmd

  def _execute(self, cmd, **kwargs):
    cmdstr = ' '.join(cmd)
    logging.info('[Executing]: %s' % cmdstr)
    kwargs['env'] = self.env
    res = subprocess.call(cmd, **kwargs)
    if res != 0:
      print('[%s] failed with exitcode %d.' % (cmdstr, res), file=sys.stderr)
      return False
    return True

  def execute_tasks(self, tasks):
    for cmd, args in tasks:
      if not self._execute(cmd, **args):
        return False
    return True


def delete_files(files, globs):
  fs = files + list(chain.from_iterable(glob.glob(g) for g in globs))
  for f in fs:
    if os.path.exists(f):
      if os.path.isfile(f):
        logging.info('Removing file: [%s]' % f)
        os.remove(f)
      elif os.path.islink(f):
        logging.info('Removing link: [%s]' % f)
        os.unlink(f)


def download_source_archive(url, arc, path, check_path=None):
  # work around for extract failures on windows
  def remove_duplicate(tf):
    names = set()
    members = []
    while True:
      member = tf.next()
      if not member:
        break
      # skip all non-files and duplicate members
      if member.isfile() and member.name not in names:
        names.add(member.name)
        members.append(member)
    return members

  check_path = check_path or path
  if os.path.exists(arc):
    logging.info('Skip downloading: %s exists.' % arc)
  else:
    print('Downloading source archive from [%s].' % url)
    try:
      rsp = urlopen(url)
    except:
      print('Failed to download source archive.', file=sys.stderr)
      raise
    with open(arc, 'wb+') as fp:
      while True:
        s = rsp.read(65536)
        if not s:
          break
        fp.write(s)
      rsp.close()
    print('Successfully downloaded soruce archive to [%s].' % arc)
  if os.path.exists(check_path):
    logging.info('Skip extracting: %s exists.' % check_path)
  else:
    logging.info('Extracting archive to %s' % check_path)
    tarfile.open(arc, mode='r|*').extractall(path, remove_duplicate(tarfile.open(arc, mode='r|*')))
    print('Successfully extracted to [%s].' % path)


def download_from_github(cwd, user, proj, commit, check_path=None):
  url = 'https://github.com/%s/%s/archive/%s.tar.gz' % (user, proj, commit)
  check_path = check_path or os.path.join(cwd, '%s-%s' % (proj, commit))
  arc = os.path.join(cwd, '%s-%s.tar.gz' % (proj, commit))
  download_source_archive(url, arc, os.path.dirname(arc), check_path)


def build_protobuf3(wd, conf):
  path = 'e61ff47ac30f7ce7b06d789f18439d5142ac8c97'
  # Need to prefix 'v' when downloading from tags
  # version = 'v' + path
  version = path
  repodir = os.path.join(wd, 'protobuf-%s' % path)
  download_from_github(wd, 'google', 'protobuf', version, repodir)

  if conf.win:
    cxxflags = '-DCMAKE_CXX_FLAGS=-DLANG_CXX11 -EHsc'
  else:
    cxxflags = '-DCMAKE_CXX_FLAGS=-fPIC -std=c++11 -DLANG_CXX11'
  cmake_cmd = conf.cmake_cmd([
    '-DBUILD_TESTING=OFF',
    cxxflags,
  ], 'cmake')

  conf.env['PROTOC'] = os.path.join(repodir, 'protoc' + conf.execext)
  workflow = [
    (['sed', '-i', 's!"/MD" "/MT"!"/MD" "/MD"!g',
      os.path.join(repodir, 'cmake', 'CMakeLists.txt')], {}),
    (cmake_cmd, {'cwd': repodir}),
    (conf.make + ['clean'], {'cwd': repodir}),
    (conf.make, {'cwd': repodir}),
    ([sys.executable, 'setup.py', 'build'], {
      'cwd': os.path.join(repodir, 'python'),
    }),
  ]
  if conf.execute_tasks(workflow):
    shutil.copy(os.path.join(repodir, 'LICENSE'), os.path.join(conf.installdir, 'LICENSE-protobuf'))
    src_include_dir = os.path.join(repodir, 'src', 'google')
    dst_include_dir = os.path.join(conf.installdir, 'include', 'google')
    if os.path.exists(dst_include_dir):
      shutil.rmtree(dst_include_dir)
    dst_pylib = os.path.join(conf.installdir, 'pylib', 'google')
    if os.path.exists(dst_pylib):
      shutil.rmtree(dst_pylib)
    pylib = os.path.join(repodir, 'python', 'build', 'lib', 'google')
    if not os.path.exists(pylib):
      # workaround for ubuntu
      pylib = os.path.join(repodir, 'python', 'build', 'lib.linux-x86_64-2.7', 'google')
    shutil.copytree(src_include_dir, dst_include_dir)
    shutil.copytree(pylib, dst_pylib)
    shutil.copy(os.path.join(repodir, 'protoc%s' % conf.execext), os.path.join(conf.installdir, 'bin'))
    shutil.copy(os.path.join(repodir, 'libprotoc%s' % conf.libext), os.path.join(conf.installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'libprotobuf%s' % conf.libext), os.path.join(conf.installdir, 'lib'))
    return True


def build_zlib(wd, conf):
  version = '1.2.8'
  extract_dir = os.path.join(wd, 'v' + version)
  archive = extract_dir + '.tar.gz'
  url = 'https://github.com/madler/zlib/archive/v%s.tar.gz' % version
  download_source_archive(url, archive, extract_dir)

  repodir = os.path.join(extract_dir, 'zlib-' + version)
  workflow = [
    (conf.cmake_cmd([]), {'cwd': repodir}),
    (conf.make + ['clean'], {'cwd': repodir}),
    (conf.make, {'cwd': repodir}),
  ]

  # ZLib seems to build debug and release at once
  def maybe_dstname(lib):
    name, ext = os.path.splitext(lib)
    is_debug = name.endswith('d')
    if is_debug and conf.debug:
      return name[:-1] + ext
    elif not is_debug and not conf.debug:
      return lib

  if conf.execute_tasks(workflow):
    for f in glob.iglob(os.path.join(repodir, '*.h')):
      shutil.copy(f, os.path.join(conf.installdir, 'include'))
    for f in glob.iglob(os.path.join(repodir, '*.lib')):
      dst = maybe_dstname(os.path.basename(f))
      if dst:
        shutil.copy(f, os.path.join(conf.installdir, 'lib', dst))
    for f in glob.iglob(os.path.join(repodir, '*.dll')):
      dst = maybe_dstname(os.path.basename(f))
      if dst:
        shutil.copy(f, os.path.join(conf.installdir, 'lib', dst))
    return True


def prepare_boringssl(wd, conf):
  version = '2490'
  repodir = os.path.join(wd, version)
  archive = repodir + '.tar.gz'
  url = 'https://boringssl.googlesource.com/boringssl/+archive/%s.tar.gz' % version
  download_source_archive(url, archive, repodir)
  if conf.win:
    subprocess.call(['sed', '-i', 's/-WX//g', os.path.join(repodir, 'CMakeLists.txt')])
  else:
    subprocess.call(['sed', '-i', 's/-Werror//g', os.path.join(repodir, 'CMakeLists.txt')])
  return version


def build_boringssl(wd, conf):
  version = prepare_boringssl(wd, conf)
  repodir = os.path.join(wd, str(version))
  if conf.win:
    cmake_args = []
  else:
    cmake_args = ['-DCMAKE_C_FLAGS=-fPIC']
  cmake_cmd = conf.cmake_cmd(cmake_args)

  workflow = [
    (cmake_cmd, {'cwd': repodir}),
    (conf.make + ['clean'], {'cwd': repodir}),
    (conf.make, {'cwd': repodir}),
  ]
  if conf.execute_tasks(workflow):
    src_include_dir = os.path.join(repodir, 'include', 'openssl')
    dst_include_dir = os.path.join(conf.installdir, 'include', 'openssl')
    if os.path.exists(dst_include_dir):
      shutil.rmtree(dst_include_dir)
    shutil.copytree(src_include_dir, dst_include_dir)
    shutil.copy(os.path.join(repodir, 'ssl', '%sssl%s' % (conf.libprefix, conf.libext)),
                os.path.join(conf.installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'crypto', '%scrypto%s' % (conf.libprefix, conf.libext)),
                os.path.join(conf.installdir, 'lib'))
    return True


class GrpcTarget(object):
  def __init__(self, name, src, deps, build, secure):
    self.name = name
    self.src = src
    self.deps = deps
    self.build = build
    self.secure = secure


def build_grpc(wd, conf):
  def find_in_json(arr, name):
    for elem in arr:
      if elem['name'] == name:
        return elem

  def collect_grpc_targets(acc, dic, name, executable=False):
    key = 'libs' if not executable else 'targets'
    if name in acc[key]:
      return
    target = find_in_json(dic[key], name)
    if not target:
      print('Not found "%s" in %s' % (name, key), file=sys.stderr)
    src = target['src']
    for fg in target.get('filegroups', []):
      src.extend(find_in_json(dic['filegroups'], fg)['src'])
    deps = target.get('deps', [])
    acc[key][name] = GrpcTarget(
        name, src, deps, target['build'], target['secure'] == 'yes')
    for dep in deps:
      collect_grpc_targets(acc, dic, dep)

  version = 'release-0_11_1'
  repodir = os.path.join(wd, 'grpc-%s' % version)
  download_from_github(wd, 'grpc', 'grpc', version)

  buildyaml = os.path.join(repodir, 'build.yaml')
  with open(buildyaml, 'r') as fp:
    dic = yaml.load(fp)
  acc = {
    'targets': {},
    'libs': {},
  }
  collect_grpc_targets(acc, dic, 'grpc++')
  collect_grpc_targets(acc, dic, 'grpc_cpp_plugin', True)

  cmake_template = os.path.join(buildenv.ROOT_DIR, 'tools', 'CMakeLists.txt.grpc.template')
  cmake_out = os.path.join(repodir, 'CMakeLists.txt')
  with open(cmake_out, 'w+') as fp:
    fp.write(mako.template.Template(filename=cmake_template).render(**acc))

  cmake_args = [
    '-DCMAKE_PREFIX_PATH=%s' % conf.installdir,
  ]
  if not conf.win:
    cmake_args.append('-DCMAKE_CXX_FLAGS=-fPIC -std=c++11')
  cmake_cmd = conf.cmake_cmd(cmake_args)

  workflow = [
    (cmake_cmd, {'cwd': repodir}),
    (conf.make + ['clean'], {'cwd': repodir}),
    (conf.make, {'cwd': repodir}),
  ]
  if conf.execute_tasks(workflow):
    include_dirs = [
      (repodir, 'grpc'),
      (repodir, 'grpc++'),
    ]
    for repo, include in include_dirs:
      src_include_dir = os.path.join(repo, 'include', include)
      dst_include_dir = os.path.join(conf.installdir, 'include', include)
      if os.path.exists(dst_include_dir):
        shutil.rmtree(dst_include_dir)
      shutil.copytree(src_include_dir, dst_include_dir)
    libs = [
      '%sgpr%s' % (conf.libprefix, conf.libext),
      '%sgrpc%s' % (conf.libprefix, conf.libext),
      '%sgrpc++%s' % (conf.libprefix, conf.libext),
    ]
    for lib in libs:
      shutil.copy(os.path.join(repodir, lib), os.path.join(conf.installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'grpc_cpp_plugin%s' % conf.execext), os.path.join(conf.installdir, 'bin'))
    return True


def clean(conf):
  wd = os.path.join(buildenv.ROOT_DIR, 'build', 'tmp')
  for e in os.listdir(wd):
    p = os.path.join(wd, e)
    if os.path.isdir(p):
      print('Removing temporary directory: ' + p)
      shutil.rmtree(p)
  # if os.path.isdir(conf.installdir):
  #   print('Removing installation directory: ' + p)
  #   shutil.rmtree(conf.installdir)
  return 0


def build(conf):
  wd = os.path.join(buildenv.ROOT_DIR, 'build', 'tmp')
  if not os.path.exists(wd):
    os.makedirs(wd)

  install_dirs = [
    conf.installdir,
    os.path.join(conf.installdir, 'bin'),
    os.path.join(conf.installdir, 'lib'),
    os.path.join(conf.installdir, 'include')
  ]
  for d in install_dirs:
    if not os.path.exists(d):
      os.makedirs(d)
  wd = os.path.join(buildenv.ROOT_DIR, 'build', 'tmp')
  if not os.path.exists(wd):
    os.makedirs(wd)

  if conf.win and not build_zlib(wd, conf):
    return -1
  if not build_protobuf3(wd, conf):
    return -1
  if not build_boringssl(wd, conf):
    return -1
  if not build_grpc(wd, conf):
    return -1
  return 0


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--shared', action='store_true', help='build shared libraries')
  p.add_argument('--clean', action='store_true', help='cleanup intermediate directory')
  p.add_argument('--jobs', '-j', type=int, default=concurrency(), help='number of concurrent compilation jobs (no effect on Windows)')
  p.add_argument('--clang', action='store_true', help='use clang')
  p.add_argument('--cc', help='C compiler')
  p.add_argument('--cxx', help='C++ compiler')
  p.add_argument('--out', '-o', default=buildenv.DEFAULT_DEPS,
                 help='installation root path for dependencies')
  p.add_argument('--verbose', '-v', action='store_true')
  args = p.parse_args(argv)
  loglvl = logging.INFO if args.verbose else logging.WARN
  logging.basicConfig(level=loglvl)

  cc = args.cc or (args.clang and 'clang')
  cxx = args.cxx or (args.clang and 'clang++')

  conf = BuildConf(cc, cxx, args.shared, False, args.jobs, os.path.join(args.out, 'Release'))
  dconf = BuildConf(cc, cxx, args.shared, True, args.jobs, os.path.join(args.out, 'Debug'))
  if args.clean:
    clean(conf)
    clean(dconf)
  else:
    build(conf)
    clean(conf)
    build(dconf)

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
