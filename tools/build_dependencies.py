#!/usr/bin/env python

import argparse
import glob
import json
import mako.template
import multiprocessing
import os
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
      fp.write(rsp.read())
    print('Successfully downloaded soruce archive to [%s].' % arc)
  if os.path.exists(check_path):
    print('Skip extracting: %s exists.' % check_path)
  else:
    print('Extracting archive.')
    tf = tarfile.open(arc, mode='r|*')
    tf.extractall(path)
    print('Successfully extracted to [%s].' % path)


def download_from_github(cwd, user, proj, commit, check_path=None):
  url = 'https://github.com/%s/%s/archive/%s.tar.gz' % (user, proj, commit)
  check_path = check_path or os.path.join(cwd, '%s-%s' % (proj, commit))
  arc = os.path.join(cwd, '%s-%s.tar.gz' % (proj, commit))
  download_source_archive(url, arc, os.path.dirname(arc), check_path)


def build_protobuf3(wd, installdir, jobs, shared, debug):
  # version = 'v3.0.0-alpha-3'
  # repodir = os.path.join(wd, 'protobuf-%s' % version)
  # # archive for protobuf tag does not have 'v' prefix
  # download_from_github(wd, 'google', 'protobuf', version, 'protobuf-3.0.0-alpha-3')

  version = '43dcbbfec7e906ede3d383a0139a05ff8a03481f'
  repodir = os.path.join(wd, 'protobuf-%s' % version)
  download_from_github(wd, 'google', 'protobuf', version)

  shared_on = 'ON' if shared else 'OFF'
  build_type = 'Debug' if debug else 'Release'
  cmake_cmd = [
    'cmake',
    '-GNinja',
    '-DBUILD_TESTING=OFF',
    '-DCMAKE_CXX_FLAGS=-fPIC -std=c++11 -DLANG_CXX11',
    '-DBUILD_SHARED_LIBS=' + shared_on,
    '-DCMAKE_BUILD_TYPE=' + build_type,
  ]

  cmake_cmd.append('cmake')

  workflow = [
    (cmake_cmd, {'cwd': repodir}),
    # (['ninja', '-C%s' % repodir, 'clean'], {}),
    (['ninja', '-j%s' % jobs, '-C%s' % repodir], {}),
  ]
  if execute_tasks(workflow):
    shutil.copy(os.path.join(repodir, 'LICENSE'), os.path.join(installdir, 'LICENSE-protobuf'))
    src_include_dir = os.path.join(repodir, 'src', 'google')
    dst_include_dir = os.path.join(installdir, 'include', 'google')
    if os.path.exists(dst_include_dir):
      shutil.rmtree(dst_include_dir)
    shutil.copytree(src_include_dir, dst_include_dir)
    shutil.copy(os.path.join(repodir, 'protoc'), os.path.join(installdir, 'bin'))
    libext = '.so' if shared else '.a'
    shutil.copy(os.path.join(repodir, 'libprotoc%s' % libext), os.path.join(installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'libprotobuf%s' % libext), os.path.join(installdir, 'lib'))
    return True


def prepare_boringssl(wd):
  version = '2357'
  repodir = os.path.join(wd, version)
  archive = repodir + '.tar.gz'
  url = 'https://boringssl.googlesource.com/boringssl/+archive/%s.tar.gz' % version
  download_source_archive(url, archive, repodir)
  # Ignore harmless warning for clang
  subprocess.call(['sed', '-i', 's/-Werror//g', os.path.join(repodir, 'CMakeLists.txt')])
  return version


def build_boringssl(wd, installdir, jobs, shared, debug):
  version = prepare_boringssl(wd)
  repodir = os.path.join(wd, str(version))
  shared_on = 'ON' if shared else 'OFF'
  build_type = 'Debug' if debug else 'Release'
  cmake_cmd = [
    'cmake',
    '-GNinja',
    '-DCMAKE_C_FLAGS=-fPIC',
    '-DBUILD_SHARED_LIBS=' + shared_on,
    '-DCMAKE_BUILD_TYPE=' + build_type,
    '.',
  ]

  workflow = [
    (cmake_cmd, {'cwd': repodir}),
    # (['ninja', '-C%s' % repodir, 'clean'], {}),
    (['ninja', '-j%s' % jobs, '-C%s' % repodir], {}),
  ]
  if execute_tasks(workflow):
    src_include_dir = os.path.join(repodir, 'include', 'openssl')
    dst_include_dir = os.path.join(installdir, 'include', 'openssl')
    if os.path.exists(dst_include_dir):
      shutil.rmtree(dst_include_dir)
    shutil.copytree(src_include_dir, dst_include_dir)
    libext = '.so' if shared else '.a'
    shutil.copy(os.path.join(repodir, 'ssl', 'libssl%s' % libext), os.path.join(installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'crypto', 'libcrypto%s' % libext), os.path.join(installdir, 'lib'))
    return True


class GrpcTarget(object):
  def __init__(self, name, src, deps, build, secure):
    self.name = name
    self.src = src
    self.deps = deps
    self.build = build
    self.secure = secure


def build_grpc(wd, installdir, jobs, shared, debug):
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

  version = '4547d503d37ade45866688dae3ce291ec8a450bd'
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

  boringssl_dir = prepare_boringssl(wd)

  cmake_template = os.path.join(buildenv.ROOT_DIR, 'tools', 'CMakeLists.txt.grpc.template')
  cmake_out = os.path.join(repodir, 'CMakeLists.txt')
  with open(cmake_out, 'w+') as fp:
    fp.write(mako.template.Template(filename=cmake_template).render(boringssl_dir=boringssl_dir, **acc))

  shared_on = 'ON' if shared else 'OFF'
  build_type = 'Debug' if debug else 'Release'
  cmake_cmd = [
    'cmake',
    '-GNinja',
    '-DCMAKE_CXX_FLAGS=-fPIC -std=c++11',
    '-DBUILD_SHARED_LIBS=' + shared_on,
    '-DCMAKE_BUILD_TYPE=' + build_type,
    '-DCMAKE_PREFIX_PATH=%s' % installdir,
    '.',
  ]

  workflow = [
    (cmake_cmd, {'cwd': repodir}),
    # (['ninja', '-C%s' % repodir, 'clean'], {}),
    (['ninja', '-j%s' % jobs, '-C%s' % repodir], {}),
  ]
  if execute_tasks(workflow):
    include_dirs = [
      (os.path.join(wd, boringssl_dir), 'openssl'),
      (repodir, 'grpc'),
      (repodir, 'grpc++'),
    ]
    for repo, include in include_dirs:
      src_include_dir = os.path.join(repo, 'include', include)
      dst_include_dir = os.path.join(installdir, 'include', include)
      if os.path.exists(dst_include_dir):
        shutil.rmtree(dst_include_dir)
      shutil.copytree(src_include_dir, dst_include_dir)
    libext = '.so' if shared else '.a'
    libs = [
      os.path.join('openssl', 'ssl', 'libssl%s' % libext),
      os.path.join('openssl', 'crypto', 'libcrypto%s' % libext),
      'libgpr%s' % libext,
      'libgrpc%s' % libext,
      'libgrpc++%s' % libext,
    ]
    for lib in libs:
      shutil.copy(os.path.join(repodir, lib), os.path.join(installdir, 'lib'))
    shutil.copy(os.path.join(repodir, 'grpc_cpp_plugin'), os.path.join(installdir, 'bin'))
    return True


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--debug', action='store_true', help='build with debug symbols')
  p.add_argument('--shared', action='store_true', help='build shared libraries')
  p.add_argument('--clean', action='store_true', help='cleanup intermediate directory')
  p.add_argument('--jobs', '-j', type=int, default=concurrency())
  p.add_argument('--out', '-o', default=buildenv.DEFAULT_DEPS,
                 help='installation root path for dependencies')
  args = p.parse_args(argv)
  buildenv.setup_env(args.out)
  installdir = args.out
  if not os.path.exists(installdir):
    os.makedirs(installdir)
    os.makedirs(os.path.join(installdir, 'bin'))
    os.makedirs(os.path.join(installdir, 'lib'))
    os.makedirs(os.path.join(installdir, 'include'))
  try:
    wd = os.path.join(buildenv.ROOT_DIR, 'build', 'tmp')
    if not os.path.exists(wd):
      os.makedirs(wd)

    if args.clean:
      for e in os.listdir(wd):
        p = os.path.join(wd, e)
        if os.path.isdir(p):
          print('Removing temporary directory: ' + p)
          shutil.rmtree(p)
      return

    prepend_common_envpaths(installdir)
    if not build_protobuf3(wd, installdir, args.jobs, args.shared, args.debug):
      return -1
    if not build_grpc(wd, installdir, args.jobs, args.shared, args.debug):
      return -1
    return 0
  except:
    print('ERROR')
    raise

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
