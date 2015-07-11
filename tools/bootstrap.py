#!/usr/bin/env python

import argparse
import os
import subprocess
import sys

import buildenv


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--generator', '-G', default='Ninja', help='CMake generator')
  p.add_argument('--configuration', '-C', default='Debug', help='CMake build type')
  p.add_argument('--qt5dir', help='directory that has Qt5<module>/Qt5<module>Config.cmake files')
  p.add_argument('--clang', action='store_true', help='use clang')
  p.add_argument('--out', '-o', default=buildenv.DEFAULT_OUT, help='build directory path')
  # p.add_argument('--qt5dir', '-Q', default='/usr/lib/qt5', help='Qt5 directory path')
  args, other_args = p.parse_known_args(argv)
  deps_dir = buildenv.DEFAULT_DEPS
  buildenv.setup_env(deps_dir)

  if not os.path.exists(args.out):
    os.makedirs(args.out)

  cmd = [
    'cmake',
    '-G%s' % args.generator,
    '-DCMAKE_BUILD_TYPE=%s' % args.configuration,
    '-DCMAKE_BUILD_TYPE=%s' % args.configuration,
    '-DCMAKE_PREFIX_PATH=%s' % deps_dir,
    # '-DPROTOBUF_PROTOC_LIBRARY=%s/lib/libprotoc.so' % deps_dir,
    # '-DPROTOBUF_PROTOC_LIBRARY_DEBUG=%s/lib/libprotoc.so' % deps_dir,
    # '-DPROTOBUF_LIBRARY=%s/lib/libprotobuf.so' % deps_dir,
    # '-DPROTOBUF_LIBRARY_DEBUG=%s/lib/libprotobuf.so' % deps_dir,
    # '-DPROTOBUF_INCLUDE_DIR=%s/include' % deps_dir,
    # '-DPROTOBUF_PROTOC_EXECUTABLE=%s/bin/protoc' % deps_dir,
    # '-DGRPC_ROOT=%s' % deps_dir,
  ]

  if args.qt5dir:
    cmd.extend([
      '-DQt5Core_DIR=%s/Qt5Core' % args.qt5dir,
      '-DQt5Qml_DIR=%s/Qt5Qml' % args.qt5dir,
      '-DQt5Quick_DIR=%s/Qt5Quick' % args.qt5dir,
      '-DQt5QuickTest_DIR=%s/Qt5QuickTest' % args.qt5dir,
    ])

  if args.clang:
    cmd.extend([
      '-DCMAKE_C_COMPILER=clang',
      '-DCMAKE_CXX_COMPILER=clang++',
    ])
  cmd.extend(other_args)
  cmd.append(buildenv.ROOT_DIR)

  return subprocess.call(cmd, cwd=args.out)

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
