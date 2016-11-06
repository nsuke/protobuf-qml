#!/usr/bin/env python

import argparse
import os
import platform
import subprocess
import sys

import buildenv


def build(configuration, out, args, cmake_args):
    deps_dir = args.dependency_root or os.path.join(
        buildenv.DEFAULT_DEPS, configuration)
    buildenv.setup_env(deps_dir)
    if not os.path.exists(out):
        os.makedirs(out)

    cmd = [
        'cmake',
        '-G%s' % args.generator,
        '-DCMAKE_BUILD_TYPE=%s' % configuration,
        '-DCMAKE_PREFIX_PATH=%s' % deps_dir,
        '-DProtobuf_DIR=%s/lib64/cmake' % deps_dir,
        '-DGRPC_ROOT=%s' % deps_dir,
    ]

    if args.qt5dir:
        cmd.extend([
            '-DQt5Core_DIR=%s/Qt5Core' % args.qt5dir,
            '-DQt5Qml_DIR=%s/Qt5Qml' % args.qt5dir,
            '-DQt5Quick_DIR=%s/Qt5Quick' % args.qt5dir,
            '-DQt5Test_DIR=%s/Qt5Test' % args.qt5dir,
            '-DQt5QuickTest_DIR=%s/Qt5QuickTest' % args.qt5dir,
        ])

    cc = args.cc or (args.clang and 'clang')
    cxx = args.cxx or (args.clang and 'clang++')
    blacklist = os.path.join(buildenv.ROOT_DIR, 'tsan-blacklist.txt')
    if cc:
        cmd.append('-DCMAKE_C_COMPILER=' + cc)
    if cxx:
        cmd.append('-DCMAKE_CXX_COMPILER=' + cxx)
    if args.tsan:
        cmd.append('-DCMAKE_CXX_FLAGS=-fsanitize=thread -fsanitize-blacklist=%s -fPIC -O1' % blacklist)
        cmd.append('-DCMAKE_CXX_FLAGS_DEBUG=-O1 -g3')
        cmd.append('-DCMAKE_CXX_FLAGS_RELEASE=-g')
        cmd.append('-DCMAKE_C_FLAGS=-fsanitize=thread -fsanitize-blacklist=%s -fPIC' % blacklist)
        cmd.append('-DCMAKE_C_FLAGS_DEBUG=-O1 -g3')
        cmd.append('-DCMAKE_C_FLAGS_RELEASE=-g')

    cmd.extend(cmake_args)
    cmd.append(buildenv.ROOT_DIR)

    print('Invoking cmake command: %s' % ' '.join(cmd))
    return subprocess.call(cmd, cwd=out)


def main(argv):
    win = platform.system() == 'Windows'
    p = argparse.ArgumentParser()
    p.add_argument('--tsan', action='store_true',
                   help='enable thread sanitizer')
    p.add_argument('--generator', '-G',
                   default='NMake Makefiles' if win else 'Ninja', help='CMake generator')
    p.add_argument(
        '--qt5dir', '-Q', help='directory that has Qt5<module>/Qt5<module>Config.cmake files')
    p.add_argument('--clang', action='store_true', help='use clang')
    p.add_argument('--cc', help='C compiler')
    p.add_argument('--cxx', help='C++ compiler')
    p.add_argument('--out', '-o', default=buildenv.DEFAULT_OUT,
                   help='build directory path')
    p.add_argument('--dependency-root', '-D')
    args, cmake_args = p.parse_known_args(argv)

    build('Debug', os.path.join(args.out, 'Debug'), args, cmake_args)
    build('Release', os.path.join(args.out, 'Release'), args, cmake_args)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
