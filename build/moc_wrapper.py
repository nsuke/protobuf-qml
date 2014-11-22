#!/usr/bin/env python
import argparse
import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)


def fix_path(path):
  return os.path.realpath(path)


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--includes', '-I')
  p.add_argument('--defines', '-D')
  p.add_argument('--moc')
  p.add_argument('--out')
  p.add_argument('input')
  args = p.parse_args()
  o = fix_path(args.out)
  d = os.path.realpath(os.path.dirname(o))
  if not os.path.exists(d):
    try:
      os.makedirs(d)
    except Exception:
      pass
  cmd = [
    fix_path(args.moc) or 'moc',
    '--o', o,
  ]
  if args.includes:
    cmd.extend(['-I%s' % i for i in args.includes.split(' ')])
  if args.defines:
    cmd.extend(['-D%s' % i for i in args.defines.split(' ')])
  cmd.append(fix_path(args.input))

  return subprocess.call(cmd)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
