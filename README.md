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

### Dependencies

You need Qt 5 installed on your system. Other than that, here are current known dependencies.

#### tools
* cmake
* make
* ninja (optional in the near future)
* Autotool (optional in the future)
* curl

#### libraries
* zlib (optional in the future)

#### python modules
* simplejson (optional in the future)
* mako (optional in the future)

Ubuntu 14.04 example for installing above dependencies:
```
# apt-get install
      python-simplejson
      python-mako
      libgoogle-perftools-dev
      curl
      build-essential
      cmake
      ninja-build
      autoconf
      libtool
      zlib1g-dev
```

### Build

The flow below is actually run on build server.
Since Protocol Buffers 3 is not available as prebuilt binary, it is automatically built in the process.
It's just load of configure, make, cmake etc under the cover.
```
$ source ./tools/setup_env.sh
$ ./tools/build_dependencies.py
$ ./tools/bootstrap.py
$ ninja -C out
```

TBD: instruction to use system protobuf3 and gRPC

Testing:
---
```
$ ./tools/run_test.py
```

Dependency versions:
---
Tested with following library versions:
* Protocol Buffers 3.0-alpha3
* Qt 5.2.1 and 5.4.2
