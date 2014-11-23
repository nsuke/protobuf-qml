import argparse
import os
import stat
import sys


def main(argv):
  p = argparse.ArgumentParser()
  p.add_argument('--ld-library-path', '-L', nargs='*')
  p.add_argument('test_runner')
  p.add_argument('root_dir')
  p.add_argument('qml_dir')
  p.add_argument('qml_test_dir')
  p.add_argument('path')
  args = p.parse_args()

  path = args.path
  with open(path, 'w') as fp:
    fp.writelines([
      '#!/usr/bin/env python\n',
      'import os\n',
      'import subprocess\n',
      'import sys\n',
      '\n',
      'if __name__ == "__main__":\n',
      '  os.environ["LD_LIBRARY_PATH"] = "%s"\n' % os.pathsep.join(args.ld_library_path),
      '  cmd = ["%s", "-import", "%s", "-input", "%s"]\n' % (
        args.test_runner, args.qml_dir, args.qml_test_dir),
      '  print(cmd)\n'
      '  res = subprocess.Popen(cmd, cwd="%s", env=os.environ).wait()\n' % (
        os.path.realpath(args.root_dir)),
      '  sys.exit(res)\n',
    ])
  os.chmod(path, os.stat(path).st_mode | stat.S_IEXEC)

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
