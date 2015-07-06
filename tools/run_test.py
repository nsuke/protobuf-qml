#!/usr/bin/env python

import argparse
import os
import subprocess
import sys

import buildenv


def run_test_server(outdir):
  cmd = [
    os.path.join(outdir, 'bin', 'test-hello-async-server'),
  ]
  return subprocess.Popen(cmd, cwd=outdir)


def run_client_test(outdir):
  cmd = [
    os.path.join(outdir, 'bin', 'test-grpc-qml'),
  ]
  return subprocess.call(cmd, cwd=outdir)


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--out', '-o', default=buildenv.DEFAULT_OUT, help='build directory path')
  args = p.parse_args(argv)
  buildenv.setup_env()
  p = run_test_server(args.out)
  try:
    return run_client_test(args.out)
  finally:
    p.kill()


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
