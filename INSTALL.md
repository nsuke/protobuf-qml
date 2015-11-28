Install
================================================================================

Note for Windows
--------------------------------------------------------------------------------

This library uses C++11 features only available for Visual C++ 2015 or later.

So basically unless you build Qt yourself, you'll need to wait for Qt 5.6.0.


Install dependencies
--------------------------------------------------------------------------------

#### Libraries

* Qt 5.5.0 or later
* Protocol Buffers 3.0 beta-1 or later (Automatically built)
* gRPC 0.11.0 or later (Automatically built)
* zlib

#### Build dependencies

* cmake
* ninja
* python
* PyYAML (Python module)
* mako (Python module)
* perl
* go

For example, if you are on Ubuntu 14.04, installation looks like following:

```
# add-apt-repository ppa:beineri/opt-qt55-trusty
# apt-get update
# apt-get install \
      zlib1g-dev \
      cmake \
      ninja-build \
      python-yaml \
      python-mako \
      perl \
      golang \
      qt55declarative
```


Build
--------------------------------------------------------------------------------

```
$ ./tools/build_dependencies.py --shared
$ ./tools/bootstrap.py
$ source ./tools/setup_env.sh
$ ninja -C out/Release
```


Install
--------------------------------------------------------------------------------

### Dependencies

Built dependencies are in *build/deps/lib* and *build/deps/bin*.

#### Protocol Buffers compiler

Put *build/deps/bin/protoc* to a directory included to *PATH* environment variable.

#### Runtime libraries

Tell OS where to find libraries. One way to do this is:

```
$ export LD_LIBRARY_PATH=$(pwd)/build/deps/lib
```

### Compiler plugin

Put *out/Release/bin/protoc-gen-qml* file to a directory included to *PATH* environment variable.

### QML module

Put *out/Release/bin/Protobuf* and *out/Release/bin/Grpc* directory to the directory
where your QML app executable resides (directly without "out/Release/bin" parent directories).

```
--- my_qml_app
 |
 -- Protobuf
 |
 -- Grpc
```

TBD: Load from qmlscene apps
