#!/bin/sh

mkdir -p out
pushd out
cmake -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_PREFIX_PATH="$HOME/dev/usr" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_FIND_ROOT_PATH=$HOME/usr/dev/lib/cmake ..
popd
