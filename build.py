#!/usr/bin/env python

import argparse
import multiprocessing
import os
import sys
import subprocess

ROOT_DIR = os.path.realpath(os.path.dirname(__file__))
DEPS_DIR = os.path.join(ROOT_DIR, 'deps')


def execute(cmd, cwd=DEPS_DIR, env=os.environ, err=None):
  try:
    if not err:
      err = 'Failed to execute "%s"\n' % ' '.join(cmd)
#  print(cmd)
#  return True
    res = subprocess.Popen(cmd, cwd=cwd, env=env).wait()
    if res != 0:
      sys.stderr.write(err)
      return False
    else:
      return True
  except Exception:
    sys.stderr.write(err)
    raise


class GitDependency(object):
  def __init__(self, name, url, ref):
    self.name = name
    self.url = url
    self.dir = os.path.join(DEPS_DIR, name)
    self.ref = self._rev_parse(ref)

  def execute(self, cmd, cwd=DEPS_DIR, err=None):
    if not err:
      err = ' '.join(cmd)
    print(err)
    return True
    # return subprocess.Popen(cmd, cwd=cwd).wait() == 0

  def _rev_parse(self, ref):
    p = subprocess.Popen(['git', 'rev-parse', ref], cwd=self.dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    o, _ = p.communicate()
    return o

  def get_source(self):
    if os.path.exists(os.path.join(self.dir, '.git')):
      current = self._rev_parse('HEAD')
      if current == self.ref:
        sys.stdout.write('Source is already up to date.\n')
        return True
      cmds = [
        sys.stdout.write('Checking out %s\n' % self.ref)
        ['git', 'fetch', 'origin'],
        ['git', 'checkout', '-f', self.ref],
      ]
      return all(execute(cmd, cwd=self.dir) for cmd in cmds)
    else:
      cmd = [
        'git',
        'clone',
        self.url,
        self.name,
      ]
      return execute(cmd)

  def prepare(self, skip_source=False):
    if not skip_source:
      self.get_source()


class CppGitDependency(GitDependency):
  def __init__(self, name, url, ref):
    super(CppGitDependency, self).__init__(name, url, ref)

  def build(self):
    try:
      jobs = multiprocessing.cpu_count()
    except NotImplementedError:
      jobs = 2
    cmds = [
      ['sh', 'autogen.sh'],
      ['./configure', '--prefix=%s' % DEPS_DIR],
      ['make', '-j%d' % jobs],
      ['make', 'install'],
    ]
    return all(execute(cmd, cwd=self.dir) for cmd in cmds)

  def prepare(self, skip_source=False):
    return super(CppGitDependency, self).prepare(skip_source) and self.build()


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--skip-tests', action='store_true', help='Do not trigger tests.')
  p.add_argument('--skip-source', action='store_true', help='Do not trigger dependency download.')
  p.add_argument('--skip-deps', action='store_true', help='Do not use downloaded dependencies.')
  p.add_argument('--gyp-includes', "-I", help='Use additional gyp include.', nargs='+')
  args = p.parse_args(argv)

  deps = [
    GitDependency('gyp', 'http://git.chromium.org/external/gyp.git', 'origin/master'),
    CppGitDependency('protobuf', 'http://github.com/google/protobuf.git', 'origin/master'),
  ]

  if not args.skip_deps:
    for dep in deps:
      dep.prepare(args.skip_source)

  gyp_cmd = [
    'gyp' if args.skip_deps else os.path.join(DEPS_DIR, 'gyp', 'gyp'),
    '--depth=%s' % ROOT_DIR,
  ]

  if not args.skip_deps:
    gyp_cmd.extend([
      '-I',
      os.path.join(DEPS_DIR, 'supplement.gypi'),
    ])

  if args.gyp_includes:
    gyp_cmd.extend(['-I%s' % i for i in args.gyp_includes])

  def ninja_cmd(conf):
    return [
      'ninja',
      '-C',
      'out/%s' % conf,
    ]

  def test_cmd(conf):
    return [
      os.path.join(ROOT_DIR, 'out', conf, 'run_tests.py')
    ]

  if args.skip_tests:
    cmds = [
      gyp_cmd,
      ninja_cmd('Debug'),
      ninja_cmd('Release'),
    ]
  else:
    cmds = [
      gyp_cmd,
      ninja_cmd('Debug'),
      test_cmd('Debug'),
      ninja_cmd('Release'),
      test_cmd('Release'),
    ]

  env = os.environ
  env['LD_LIBRARY_PATH'] = os.path.join(DEPS_DIR, 'lib')
  res = all(execute(cmd, cwd=ROOT_DIR, env=env) for cmd in cmds)
  return 0 if res else 1


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
