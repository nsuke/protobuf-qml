import logging
import platform
import os

ROOT_DIR = os.path.realpath(os.path.dirname(os.path.dirname(__file__)))
DEFAULT_DEPS = os.path.join(ROOT_DIR, 'build', 'deps')
DEFAULT_OUT = os.path.join(ROOT_DIR, 'out')


logger = logging.getLogger(__name__)


def prepend_envvar(env, k, v, sep=os.pathsep):
  old = env.get(k)
  env[k] = sep.join([v, old]) if old else v


# def disable_werror(env, warns):
#   for w in warns:
#     prepend_envvar(env, 'CPPFLAGS', '-Wno-error=%s' % w, ' ')


def prepend_libdir(env, libdir):
  if platform.system() == 'Windows':
    prepend_envvar(env, 'PATH', libdir)
    prepend_envvar(env, 'LIB', libdir)
    prepend_envvar(env, 'LIBPATH', libdir)
  else:
    prepend_envvar(env, 'LD_LIBRARY_PATH', libdir)
    prepend_envvar(env, 'LIBRARY_PATH', libdir)
    prepend_envvar(env, 'LDFLAGS', '-L%s' % libdir, ' ')


def prepend_include_dir(env, include):
  if platform.system() == 'Windows':
    prepend_envvar(env, 'INCLUDE', include)
  else:
    prepend_envvar(env, 'C_INCLUDE_PATH', include)
    prepend_envvar(env, 'CPLUS_INCLUDE_PATH', include)


def setup_env(root, env=os.environ):
  logging.info('Setting up environment variable for root directory [%s]' % root)
  prepend_envvar(env, 'PATH', os.path.join(root, 'bin'))
  prepend_include_dir(env, os.path.join(root, 'include'))
  prepend_libdir(env, os.path.join(root, 'lib'))
  for k, v in env.items():
    logging.debug('%s: %s' % (k ,v))
