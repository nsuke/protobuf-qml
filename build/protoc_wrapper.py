#!/usr/bin/env python
import argparse
import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)


def fix_path(path):
  return path if os.path.isabs(path) else os.path.realpath(os.path.join(ROOT_DIR, path))


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--protoc', '-c')
  p.add_argument('--plugin', '-p')
  p.add_argument('--out', '-o')
  p.add_argument('input')
  args = p.parse_args()
  i = fix_path(args.input)
  cmd = [
    fix_path(args.protoc) or 'protoc',
    '--proto_path=%s' % os.path.dirname(i),
    '--qml_out=%s' % fix_path(args.out or '.'),
  ]
  if args.plugin:
    cmd.append('--plugin=%s' % args.plugin)
  cmd.append(i)
  return subprocess.call(cmd)

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
