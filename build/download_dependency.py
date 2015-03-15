#!/usr/bin/env python

import argparse
import multiprocessing
import os
import string
import subprocess
import sys

ROOT_DIR = os.path.realpath(os.path.dirname(__file__))
DEPS_DIR = os.path.join(ROOT_DIR, 'deps')


def execute(cmd, cwd=DEPS_DIR, env=os.environ, err=None, err_expr=['error:', 'fatal:']):
  try:
    if not err:
      err = 'Failed to execute "%s"\n' % ' '.join(cmd)
#  print(cmd)
#  return True
    p = subprocess.Popen(cmd, cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    o, e = p.communicate()
    sys.stderr.write(e)
    sys.stdout.write(o)
    failed = p.returncode != 0 or any(e.find(expr) != -1 for expr in err_expr)
    if failed:
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
    self.ref = ref
    self.source_updated = False

  def _rev_parse(self, ref):
    p = subprocess.Popen(['git', 'rev-parse', ref], cwd=self.dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    o, e = p.communicate()
    if p.returncode != 0 or e.find('fatal:') != -1:
      raise Exception('Failed to parse git rev.')
    return string.strip(o)

  def get_source(self):
    target = None
    if not os.path.exists(os.path.join(self.dir, '.git')):
      cmd = [
        'git',
        'clone',
        self.url,
        self.name,
      ]
      if not execute(cmd):
        sys.stderr.write('Failed to clone source repository.\n')
        return False
    else:
      try:
        res = execute(['git', 'cat-file', '-t', self.ref])
      except Exception:
        pass
      if not res:
        sys.stdout.write('Fetching git remote.\n')
        cmd = ['git', 'fetch', '--all']
        # TODO: If target is branch, we should fetch unconditionally
        if not execute(cmd, cwd=self.dir):
          sys.stderr.write('Failed to fetch source repository.\n')
          return False
    if not target:
      target = self._rev_parse(self.ref)
    current = self._rev_parse('HEAD')
    if current == target:
      sys.stdout.write('Local source is up to date.\n')
      return True
    sys.stdout.write('Checking out %s\n' % target)
    self.source_updated = True
    cmd = ['git', 'checkout', '-f', target]
    return execute(cmd, cwd=self.dir)

  def prepare(self):
    return self.get_source()


class CppGitDependency(GitDependency):
  def __init__(self, name, url, ref):
    super(CppGitDependency, self).__init__(name, url, ref)

  def build(self, jobs):
    try:
      jobs = jobs or multiprocessing.cpu_count()
    except NotImplementedError:
      jobs = 2
    cmds = [
      ['sh', 'autogen.sh'],
      ['./configure', '--prefix=%s' % DEPS_DIR],
      ['make', '-j%d' % jobs],
      ['make', 'install'],
    ]
    return all(execute(cmd, cwd=self.dir) for cmd in cmds)

  def prepare(self, jobs):
    if not super(CppGitDependency, self).prepare():
      return False
    if not self.source_updated and os.path.exists(os.path.join(DEPS_DIR, 'bin')):
      return True
    return self.build(jobs)


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--jobs', '-j', type=int)
  args = p.parse_args(argv)
  if not os.path.exists(DEPS_DIR):
    os.makedirs(DEPS_DIR)
  d = CppGitDependency('protobuf', 'https://github.com/google/protobuf.git', '9d31c227a3ab0260c3e6ebc7e1a555f47582dab4')
  res = d.prepare(args.jobs)
  return 0 if res else 1


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
