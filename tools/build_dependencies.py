#!/usr/bin/env python

import argparse
import glob
import json
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
  print('Executing %s' % cmdstr)
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


def prepend_common_envpaths(installdir):
  prepend_env('PATH', os.path.join(installdir, 'bin'))
  prepend_env('LD_LIBRARY_PATH', os.path.join(installdir, 'lib'))
  prepend_env('LIBRARY_PATH', os.path.join(installdir, 'lib'))
  prepend_env('LDFLAGS', '-L%s' % os.path.join(installdir, 'lib'), ' ')
  prepend_env('C_INCLUDE_PATH', os.path.join(installdir, 'include'))
  prepend_env('CPLUS_INCLUDE_PATH', os.path.join(installdir, 'include'))


def prepare_repo(repodir, repourl, refspec):
  if not os.path.exists(repodir):
    os.makedirs(repodir)
  workflow = []
  if not os.path.exists(os.path.join(repodir, '.git')):
    workflow.extend([
      (['git', 'init'], {'cwd': repodir}),
      (['git', 'remote', 'add', 'origin', repourl], {'cwd': repodir}),
    ])
  workflow.append((['git', 'fetch', '--depth=1', 'origin', refspec], {'cwd': repodir}))
  workflow.append((['git', 'checkout', '-f', 'origin/%s' % refspec], {'cwd': repodir}))
  execute_tasks(workflow)


def download_from_github(cwd, user, proj, commit, path=None):
  path = path or os.path.join(cwd, '%s-%s' % (proj, commit))
  arc = path + '.tar.gz'
  if not os.path.exists(arc):
    url = 'https://github.com/%s/%s/archive/%s.tar.gz' % (user, proj, commit)
    print('Downloading source archive from [%s].' % url)
    try:
      rsp = urllib2.urlopen(url)
    except:
      print('Failed to download source archive.')
      raise
    with open(arc, 'wb+') as fp:
      fp.write(rsp.read())
    print('Successfully downloaded soruce archive to [%s].' % arc)
  if not os.path.exists(path):
    print('Extracting archive.')
    tf = tarfile.open(arc, mode='r|*')
    tf.extractall(os.path.dirname(path))
    print('Successfully extracted to [%s].' % path)
  return path


def build_protobuf3(wd, installdir, jobs):
  # repodir = os.path.join(wd, 'protobuf')
  # installdir = os.path.join(wd, 'deps', 'protobuf3')
  # prepare_repo(repodir, 'https://github.com/google/protobuf.git', 'master')
  repodir = os.path.join(wd, 'protobuf-3.0.0-alpha-3')
  repodir = download_from_github(wd, 'google', 'protobuf', 'v3.0.0-alpha-3', repodir)
  workflow = [
    (['./autogen.sh'], {'cwd': repodir}),
    (['./configure', '--disable-static', '-prefix', installdir], {'cwd': repodir}),
    (['make', '-j%d' % jobs, '-C', repodir], {}),
    (['make', '-C', repodir, 'install'], {}),
  ]
  res = execute_tasks(workflow)
  delete_files([], [os.path.join(installdir, 'lib', 'libprotobuf-lite.*')])
  shutil.copy2(os.path.join(repodir, 'LICENSE'), os.path.join(installdir, 'LICENSE-protobuf'))
  prepend_common_envpaths(installdir)
  return res


def build_grpc(wd, installdir, jobs):
  # repodir = os.path.join(wd, 'grpc')
  # installdir = os.path.join(wd, 'deps', 'grpc')
  # prepare_repo(repodir, 'https://github.com/grpc/grpc.git', 'master')
  repodir = download_from_github(wd, 'grpc', 'grpc', '0ee84dc10feb0eeddc304d18638bbcf3faff8f04')

  buildjson = os.path.join(repodir, 'build.json')
  buildjson_orig = os.path.join(repodir, 'build.json.orig')
  if not os.path.exists(buildjson_orig):
    shutil.move(buildjson, buildjson_orig)
  with open(buildjson_orig, 'r') as orig:
    dic = json.load(orig)
  targets = []
  targets_orig = dic['targets']
  nobuild = ['test', 'benchmark', 'do_not_build']
  nobuild_langs = ['csharp', 'python', 'ruby', 'objective_c']
  for t in targets_orig:
    if t['build'] not in nobuild and t['language'] not in nobuild_langs and all([l not in t['name'] for l in nobuild_langs]):
      targets.append(t)
  dic['targets'] = targets

  with open(buildjson, 'w') as o:
    json.dump(dic, o)

  # openssl_url = 'https://github.com/openssl/openssl.git'
  # openssl_refspec = 'OpenSSL_1_0_2-stable'
  # prepare_repo(os.path.join(repodir, 'third_party', 'openssl'), openssl_url, openssl_refspec)
  dlpath = download_from_github(wd, 'openssl', 'openssl', 'OpenSSL_1_0_2-stable')
  gitpath = os.path.join(repodir, 'third_party', 'openssl')
  gitparent = os.path.dirname(gitpath)
  if os.path.exists(gitparent):
    shutil.rmtree(gitparent)
  if not os.path.exists(gitpath):
    shutil.copytree(dlpath, gitpath)
  vsdir = os.path.join(repodir, 'templates', 'vsprojects')
  if os.path.exists(vsdir):
    shutil.rmtree(vsdir)
  workflow = [
    # (['git', 'submodule', 'update', '--init', '--depth=1', 'third_party/zlib'], {'cwd': repodir}),
    # (['git', 'submodule', 'init', 'third_party/openssl'], {'cwd': repodir}),
    # (['git', 'clone', '--depth=1', '-b', openssl_refspec, '--separate-git-dir', os.path.join(repodir, '.git', 'modules', 'openssl'), openssl_url], {'cwd': repodir}),
    ([os.path.join(repodir, 'tools', 'buildgen', 'generate_projects.sh')], {'cwd': repodir}),
    (['make', 'clean', '-C', repodir], {}),
    (['make', 'run_dep_checks', '-C', repodir], {}),
    (['make', '-j%d' % jobs, '-C', repodir], {}),
    (['make', '-C', repodir, 'install', 'prefix=%s' % installdir], {}),
  ]
  res = execute_tasks(workflow)
  delete_files([
    os.path.join(installdir, 'bin', 'grpc_csharp_plugin'),
    os.path.join(installdir, 'bin', 'grpc_objective_c_plugin'),
    os.path.join(installdir, 'bin', 'grpc_python_plugin'),
    os.path.join(installdir, 'bin', 'grpc_ruby_plugin'),
  ], [
    os.path.join(installdir, 'lib', '*_unsecure*'),
    os.path.join(installdir, 'lib', '*.a'),
  ])
  shutil.copy2(os.path.join(repodir, 'LICENSE'), os.path.join(installdir, 'LICENSE-grpc'))
  shutil.copy2(os.path.join(repodir, 'third_party', 'openssl', 'LICENSE'), os.path.join(installdir, 'LICENSE-openssl'))

  prepend_common_envpaths(installdir)
  return res


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--jobs', '-j', type=int, default=concurrency())
  p.add_argument('--out', '-o', default=buildenv.DEFAULT_DEPS,
                 help='installation root path for dependencies')
  args = p.parse_args(argv)
  buildenv.setup_env(args.out)
  try:
    disable_werror([
      'missing-field-initializers',
      'old-style-declaration',
    ])
    wd = os.path.join(buildenv.ROOT_DIR, 'build', 'tmp')
    installdir = args.out
    if not os.path.exists(wd):
      os.makedirs(wd)
    build_protobuf3(wd, installdir, args.jobs)
    build_grpc(wd, installdir, args.jobs)
    return 0
  except:
    print('ERROR')
    raise

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
