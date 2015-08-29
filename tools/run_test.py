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
  print('Invoking command: ' + ' '.join(cmd))
  return subprocess.call(cmd, cwd=outdir)


def main(argv):
  default_conf = 'Debug' if os.path.basename(os.getcwd()) == 'Debug' else 'Release'
  p = argparse.ArgumentParser()
  p.add_argument('--configuration', '-C', default=default_conf)
  p.add_argument('--out', '-o', default=buildenv.DEFAULT_OUT, help='build directory path')
  args = p.parse_args(argv)
  buildenv.setup_env(os.path.join(buildenv.DEFAULT_DEPS, args.configuration))
  out = os.path.join(args.out, args.configuration)
  p = run_test_server(out)
  try:
    return run_client_test(out)
  finally:
    p.kill()


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
