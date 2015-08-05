Install
===

TBD: Windows

## Install dependencies
#### Libraries
* Qt 5.5.0 or later
* Protocol Buffers 3.0 alpha-3.1 or later (Automatically built)
* gRPC (Automatically built)
* zlib

#### Build dependencies
* cmake
* ninja
* python
* mako (Python module)
* perl
* go

For example, if you are on Ubuntu 14.04, installation looks like following:
```
# add-apt-repository ppa:beineri/opt-qt55-trusty
# apt-get update
# apt-get install
      zlib1g-dev
      cmake
      ninja-build
      python-mako
      perl
      golang
      qt55declarative
```

## Build
```
$ source ./tools/setup_env.sh
$ ./tools/build_dependencies.py --shared
$ ./tools/bootstrap.py
$ ninja -C out
```
## Install

### Dependencies

Built dependencies are in *build/deps/lib* and *build/deps/bin*.

#### Protocol Buffers compiler

Put *build/deps/bin/protoc* to a directory included to *PATH* environment variable.

#### Runtime libraries

Tell OS where to find libraries. One way to do it is:

```
$ export LD_LIBRARY_PATH=$(pwd)/build/deps/lib
```

### Compiler plugin

Put *out/bin/protoc-gen-qml* file to a directory included to *PATH* environment variable.

### QML module

Put *out/bin/Protobuf* and *out/bin/Grpc* directory to the directory where your QML app executable resides (directly without "out/bin" parent directories).

```
--- my_qml_app
 |
 -- Protobuf
 |
 -- Grpc
```

TBD: Load from qmlscene apps