#!/usr/bin/env python

import argparse
import os
import subprocess
import sys

import buildenv


def run_test_server(outdir):
  cmd = [
    os.path.join(outdir, 'lib', 'grpc', 'test', 'hello-server'),
  ]
  return subprocess.Popen(cmd, cwd=outdir)


def run_protobuf_test(outdir):
  cmd = [
    os.path.join(outdir, 'protobuf-qml-test'),
    '-import', os.path.join(outdir, 'plugins'),
    '-input', os.path.join(outdir, 'test'),
  ]
  return subprocess.call(cmd, cwd=outdir)


def run_client_test(outdir):
  cmd = [
    os.path.join(outdir, 'protobuf-qml-test'),
    '-import', os.path.join(outdir, 'plugins'),
    '-input', os.path.join(outdir, 'lib', 'grpc', 'test'),
  ]
  return subprocess.call(cmd, cwd=outdir)


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--out', '-o', default=buildenv.DEFAULT_OUT, help='build directory path')
  args = p.parse_args(argv)
  buildenv.setup_env()
  p = run_test_server(args.out)
  try:
    r = run_protobuf_test(args.out)
    return run_client_test(args.out) if r == 0 else r
  finally:
    p.kill()


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
