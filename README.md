protobuf-qml
============
Protocol Buffers integration for QML (QtQuick2)
---
[![Build Status](https://travis-ci.org/nsuke/protobuf-qml.svg?branch=master)](https://travis-ci.org/nsuke/protobuf-qml)

Getting started
---
This project contains protoc generator plugin executable and QML plugin.

#### Step1. use generator plugin on your .proto file to generate QML (Javascript) types
```
$ protoc --plugin /path/to/protoc-gen-qml --qml_out <output dir> <.proto file>
```
#### Step2. use QML plugin and generated types from your program

Put [qt.conf](http://doc.qt.io/qt-5/qt-conf.html) to the directory where your program executable resides with following content.
```
[plugins]
relative/path/to/qml-directory
```

Currently no biniary distribution is available yet.

Buiding:
---
You need Qt 5 installed on your system.
```
$ build/download_dependency.py
$ mkdir -p cmake_build && cd cmake_build
$ cmake \
    -DPROTOBUF_PROTOC_LIBRARY=../build/deps/lib/libprotoc.so \
    -DPROTOBUF_LIBRARY=../build/deps/lib/libprotobuf.so \
    -DPROTOBUF_INCLUDE_DIR=../build/deps/include \
    -DPROTOBUF_PROTOC_EXECUTABLE=../build/deps/bin/protoc \
    ..
$ make
```

Dependency versions:
----
Tested with following library versions:
* Protocol Buffers 3.0 snapshot
* Qt 5.2 and 5.5 snapshot
