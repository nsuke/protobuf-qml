#!/usr/bin/env python

import argparse
import glob
import json
import mako.template
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
import tarfile
import urllib2
from itertools import chain

import buildenv


def concurrency():
  try:
    return multiprocessing.cpu_count()
  except NotImplementedError:
    return 4


class BuildConf(object):
  def __init__(self, cc, cxx, shared, debug, jobs=1):
    self.win = platform.system() == 'Windows'
    self.make = ['nmake'] if self.win else ['ninja', '-j', '%d' % jobs]
    self.cc = cc
    self.cxx = cxx
    self.shared = shared
    self.debug = debug
    if self.win:
      self.libprefix = ''
      self.libext = '.dll' if shared else '.lib'
      self.execext = '.exe'
    else:
      self.libprefix = 'lib'
      self.libext = '.so' if shared else '.a'
      self.execext = ''
    self.shared_on = 'ON' if self.shared else 'OFF'
    self.build_type = 'Debug' if self.debug else 'Release'

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


def execute(cmd, **kwargs):
  cmdstr = ' '.join(cmd)
  print('Executing: %s' % cmdstr)
  res = subprocess.call(cmd, **kwargs)
  if res != 0:
    print('[%s] failed with exitcode %d.' % (cmdstr, res))
    return False
  return True


def execute_tasks(tasks):
  for cmd, args in tasks:
    if not execute(cmd, **args):
      return False
  return True


def delete_files(files, globs):
  fs = files + list(chain.from_iterable(glob.glob(g) for g in globs))
  for f in fs:
    if os.path.exists(f):
      if os.path.isfile(f):
        print('Removing file: [%s]' % f)
        os.remove(f)
      elif os.path.islink(f):
        print('Removing link: [%s]' % f)
        os.unlink(f)


def prepend_env(key, value, sep=os.pathsep):
  paths = [value]
  orig = os.getenv(key)
  if orig:
    paths.append(orig)
  os.environ[key] = sep.join(paths)


def disable_werror(warns):
  for w in warns:
    prepend_env('CPPFLAGS', '-Wno-error=%s' % w, ' ')


def prepend_libdir(libdir):
  prepend_env('LD_LIBRARY_PATH', libdir)
  prepend_env('LIBRARY_PATH', libdir)
  prepend_env('LDFLAGS', '-L%s' % libdir, ' ')


def prepend_include_dir(include):
  prepend_env('C_INCLUDE_PATH', include)
  prepend_env('CPLUS_INCLUDE_PATH', include)


def prepend_common_envpaths(installdir):
  prepend_env('PATH', os.path.join(installdir, 'bin'))
  prepend_libdir(os.path.join(installdir, 'lib'))
  prepend_include_dir(os.path.join(installdir, 'include'))


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
    print('Skip downloading: %s exists.' % arc)
  else:
    print('Downloading source archive from [%s].' % url)
    try:
      rsp = urllib2.urlopen(url)
    except:
      print('Failed to download source archive.')
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
    print('Skip extracting: %s exists.' % check_path)
  else:
    print('Extracting archive.')
    tarfile.open(arc, mode='r|*').extractall(path, remove_duplicate(tarfile.open(arc, mode='r|*')))
    print('Successfully extracted to [%s].' % path)


def download_from_github(cwd, user, proj, commit, check_path=None):
  url = 'https://github.com/%s/%s/archive/%s.tar.gz' % (user, proj, commit)
  check_path = check_path or os.path.join(cwd, '%s-%s' % (proj, commit))
  arc = os.path.join(cwd, '%s-%s.tar.gz' % (proj, commit))
  download_source_archive(url, arc, os.path.dirname(arc), check_path)


def build_protobuf3(wd, installdir, conf):
  # version = 'v3.0.0-alpha-3'
  # repodir = os.path.join(wd, 'protobuf-%s' % version)
  # # archive for protobuf tag does not have 'v' prefix
  # download_from_github(wd, 'google', 'protobuf', version, 'protobuf-3.0.0-alpha-3')

  version = '43dcbbfec7e906ede3d383a0139a05ff8a03481f'
  repodir = os.path.join(wd, 'protobuf-%s' % version)
  download_from_github(wd, 'google', 'protobuf', version)

  if conf.win:
    cxxflags = '-DCMAKE_CXX_FLAGS=-DLANG_CXX11 -EHsc'
  else:
    cxxflags = '-DCMAKE_CXX_FLAGS=-fPIC -std=c++11 -DLANG_CXX11'
  cmake_cmd = conf.cmake_cmd([
    '-DBUILD_TESTING=OFF',
    cxxflags,
  ], 'cmake')

  workflow = [
    (cmake_cmd, {'cwd': repodir}),
    (conf.make + ['clean'], {'cwd': repodir}),
    (conf.make, {'cwd': repodir}),
  ]
  if execute_tasks(workflow):
    shutil.copy(os.path.join(repodir, 'LICENSE'), os.path.join(installdir, 'LICENSE-protobuf'))
    src_include_dir = os.path.join(repodir, 'src', 'google')
    dst_include_dir = os.path.join(installdir, 'include', 'google')
    if os.path.exists(dst_include_dir):
      shutil.rmtree(dst_include_dir)
    shutil.copytree(src_include_dir, dst_include_dir)
    shutil.copy(os.path.join(repodir, 'protoc%s' % conf.execext), os.path.join(installdir, 'bin'))
    shutil.copy(os.path.join(repodir, 'libprotoc%s' % conf.libext), os.path.join(installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'libprotobuf%s' % conf.libext), os.path.join(installdir, 'lib'))
    return True


def build_zlib(wd, installdir, conf):
  version = '1.2.8'
  extract_dir = os.path.join(wd, 'v' + version)
  archive = extract_dir + '.tar.gz'
  url = 'https://github.com/madler/zlib/archive/v%s.tar.gz' % version
  download_source_archive(url, archive, extract_dir)

  repodir = os.path.join(extract_dir, 'zlib-' + version)
  workflow = [
    (conf.make + [
      '-f',
      'win32/Makefile.msc',
      'AS=ml64',
      'LOC=-DASMV -DASMINF -I.',
      'OBJA=inffasx64.obj gvmat64.obj inffas8664.obj',
    ], {'cwd': repodir}),
  ]
  if execute_tasks(workflow):
    for f in glob.iglob(os.path.join(repodir, '*.h')):
      shutil.copy(f, os.path.join(installdir, 'include'))
    for f in glob.iglob(os.path.join(repodir, '*.lib')):
      shutil.copy(f, os.path.join(installdir, 'lib'))
    for f in glob.iglob(os.path.join(repodir, '*.dll')):
      shutil.copy(f, os.path.join(installdir, 'lib'))
    return True


def prepare_boringssl(wd, conf):
  version = '685402fadd8e90f1cd70ded7f7590600128d7d89'
  repodir = os.path.join(wd, version)
  archive = repodir + '.tar.gz'
  url = 'https://boringssl.googlesource.com/boringssl/+archive/%s.tar.gz' % version
  download_source_archive(url, archive, repodir)
  if conf.win:
    subprocess.call(['sed', '-i', 's/-WX//g', os.path.join(repodir, 'CMakeLists.txt')])
  else:
    subprocess.call(['sed', '-i', 's/-Werror//g', os.path.join(repodir, 'CMakeLists.txt')])
  return version


def build_boringssl(wd, installdir, conf):
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
  if execute_tasks(workflow):
    src_include_dir = os.path.join(repodir, 'include', 'openssl')
    dst_include_dir = os.path.join(installdir, 'include', 'openssl')
    if os.path.exists(dst_include_dir):
      shutil.rmtree(dst_include_dir)
    shutil.copytree(src_include_dir, dst_include_dir)
    shutil.copy(os.path.join(repodir, 'ssl', '%sssl%s' % (conf.libprefix, conf.libext)),
                os.path.join(installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'crypto', '%scrypto%s' % (conf.libprefix, conf.libext)),
                os.path.join(installdir, 'lib'))
    return True


class GrpcTarget(object):
  def __init__(self, name, src, deps, build, secure):
    self.name = name
    self.src = src
    self.deps = deps
    self.build = build
    self.secure = secure


def build_grpc(wd, installdir, conf):
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
      print('Not found "%s" in %s' % (name, key))
    src = target['src']
    for fg in target.get('filegroups', []):
      src.extend(find_in_json(dic['filegroups'], fg)['src'])
    deps = target.get('deps', [])
    acc[key][name] = GrpcTarget(
        name, src, deps, target['build'], target['secure'] == 'yes')
    for dep in deps:
      collect_grpc_targets(acc, dic, dep)

  version = '5c575dd6e4b01cd68cca5d1917b58023dcf4ca0f'
  repodir = os.path.join(wd, 'grpc-%s' % version)
  download_from_github(wd, 'grpc', 'grpc', version)

  buildjson = os.path.join(repodir, 'build.json')

  with open(buildjson, 'r') as fp:
    dic = json.load(fp)
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
    '-DCMAKE_PREFIX_PATH=%s' % installdir,
  ]
  if not conf.win:
    cmake_args.append('-DCMAKE_CXX_FLAGS=-fPIC -std=c++11')
  cmake_cmd = conf.cmake_cmd(cmake_args)

  workflow = [
    (cmake_cmd, {'cwd': repodir}),
    (conf.make + ['clean'], {'cwd': repodir}),
    (conf.make, {'cwd': repodir}),
  ]
  if execute_tasks(workflow):
    include_dirs = [
      (repodir, 'grpc'),
      (repodir, 'grpc++'),
    ]
    for repo, include in include_dirs:
      src_include_dir = os.path.join(repo, 'include', include)
      dst_include_dir = os.path.join(installdir, 'include', include)
      if os.path.exists(dst_include_dir):
        shutil.rmtree(dst_include_dir)
      shutil.copytree(src_include_dir, dst_include_dir)
    libs = [
      '%sgpr%s' % (conf.libprefix, conf.libext),
      '%sgrpc%s' % (conf.libprefix, conf.libext),
      '%sgrpc++%s' % (conf.libprefix, conf.libext),
    ]
    for lib in libs:
      shutil.copy(os.path.join(repodir, lib), os.path.join(installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'grpc_cpp_plugin%s' % conf.execext), os.path.join(installdir, 'bin'))
    return True


def build(conf, installdir, clean):
  buildenv.setup_env(installdir)
  installdir = installdir
  if not os.path.exists(installdir):
    os.makedirs(installdir)
    os.makedirs(os.path.join(installdir, 'bin'))
    os.makedirs(os.path.join(installdir, 'lib'))
    os.makedirs(os.path.join(installdir, 'include'))
  try:
    wd = os.path.join(buildenv.ROOT_DIR, 'build', 'tmp')
    if not os.path.exists(wd):
      os.makedirs(wd)

    if clean:
      for e in os.listdir(wd):
        p = os.path.join(wd, e)
        if os.path.isdir(p):
          print('Removing temporary directory: ' + p)
          shutil.rmtree(p)
      return 0

    prepend_common_envpaths(installdir)
    if conf.win and not build_zlib(wd, installdir, conf):
      return -1
    if not build_protobuf3(wd, installdir, conf):
      return -1
    if not build_boringssl(wd, installdir, conf):
      return -1
    if not build_grpc(wd, installdir, conf):
      return -1
    return 0
  except:
    print('ERROR')
    raise


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
  args = p.parse_args(argv)

  cc = args.cc or (args.clang and 'clang')
  cxx = args.cxx or (args.clang and 'clang++')
  conf = BuildConf(cc, cxx, args.shared, False, args.jobs)
  res = build(conf, os.path.join(args.out, 'Release'), args.clean)
  if res != 0:
    return res
  conf.debug = True
  return build(conf, os.path.join(args.out, 'Debug'), args.clean)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
