protobuf-qml
============
Protocol Buffers and gRPC integration for QML (QtQuick2)
---
[![Build Status](https://travis-ci.org/nsuke/protobuf-qml.svg?branch=master)](https://travis-ci.org/nsuke/protobuf-qml)

Getting started
---
This project contains protoc generator plugin executable and QML plugins.

#### Step1. use generator plugin on your .proto file to generate QML (Javascript) types
```
$ protoc --plugin /path/to/protoc-gen-qml --qml_out <output dir> <my_proto_file.proto>
```
This will generate Javascript and QML files to &lt;output dir&gt; directory.

#### Step2. use QML plugins to import generated types to your application

Put QML plugin directories (built below) to the same directory as your application executable.

Then you can import the generated files above to any QML files.

```
import 'my_proto_file.pb.js' as MyGeneratedTypes
```
TBD: tutorials

Buiding
---

### Dependencies
#### Libraries
* Qt5
* Protocol Buffers 3 (Automatically built)
* gRPC (Optional. Automatically built)
* zlib (For gRPC only)

#### Build dependencies
* cmake
* ninja
* python
* mako (Python module) (For gRPC only)
* perl (For BoringSSL for gRPC only)

Ubuntu 14.04 example for installing above dependencies:
```
# apt-get install
      zlib1g-dev
      cmake
      ninja-build
      python-mako
      perl
      qtbase5-dev
      qtdeclarative5-dev
      qttools5-dev-tools
      qtdeclarative5-dev-tools
      qtdeclarative5-qtquick2-plugin
      qtdeclarative5-test-plugin
```

### Build
```
$ source ./tools/setup_env.sh
$ ./tools/build_dependencies.py
$ ./tools/bootstrap.py
$ ninja -C out
```
TBD: Windows and Android

TBD: instruction to use system protobuf3 and gRPC

Testing
---
```
$ cd out && ctest -VV
```

Dependency versions
---
Tested with following library versions:
* Protocol Buffers 3.0-alpha3
* Qt 5.2.1 and 5.4.2
