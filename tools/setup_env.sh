#!/bin/bash
#
# source this to setup envvars needed to build and run artifacts
#

CONFIGURATION=$1
DEPS_DIR=$2

if [ -z $CONFIGURATION ] ; then
  CUR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
  CONFIGURATION=Release
fi

if [ -z $DEPS_DIR ] ; then
  CUR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
  DEPS_DIR=$CUR/../build/deps/$CONFIGURATION
fi

export PATH=$DEPS_DIR/bin:$PATH
# -L ldflag is needed by clang
export LDFLAGS="-L$DEPS_DIR/lib $LDFLAGS"
export LIBRARY_PATH=$DEPS_DIR/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=$DEPS_DIR/lib:$LD_LIBRARY_PATH
export C_INCLUDE_DIR=$DEPS_DIR/include:$C_INCLUDE_DIR
export CPLUS_INCLUDE_DIR=$DEPS_DIR/include:$CPLUS_INCLUDE_DIR
export PYTHONPATH=$PYTHONPATH:$DEPS_DIR/pylib
