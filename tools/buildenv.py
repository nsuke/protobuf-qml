import os

ROOT_DIR = os.path.realpath(os.path.dirname(os.path.dirname(__file__)))
DEFAULT_DEPS = os.path.join(ROOT_DIR, 'build', 'deps')
DEFAULT_OUT = os.path.join(ROOT_DIR, 'out')


def prepend_envvar(k, v, sep=os.pathsep):
  old = os.environ.get(k)
  os.environ[k] = sep.join([v, old]) if old else v


def setup_env(libdir=DEFAULT_DEPS):
  prepend_envvar('PATH', os.path.join(libdir, 'bin'))
  prepend_envvar('LDFLAGS', '-L' + os.path.join(libdir, 'lib'), ' ')
  prepend_envvar('LIBRARY_PATH', os.path.join(libdir, 'lib'))
  prepend_envvar('LD_LIBRARY_PATH', os.path.join(libdir, 'lib'))
  prepend_envvar('C_INCLUDE_DIR', os.path.join(libdir, 'include'))
  prepend_envvar('CPLUS_INCLUDE_DIR', os.path.join(libdir, 'include'))
