protobuf-qml
============
Protocol Buffers integration for QML (QtQuick2)
---
[![Build Status](https://travis-ci.org/nsuke/protobuf-qml.svg?branch=master)](https://travis-ci.org/nsuke/protobuf-qml)

Supported platforms
----
Currently build configuration is tested only on Linux.
No Linux/Posix specific code is present though.

Tested with following library versions:
* Protocol Buffers 3.0
* Qt 5.0 and 5.4

Buiding:
---

Following command will download some dependencies, build, and run tests.
You need Qt5 installed on your system.

    ./build.py

If your Qt5 installation is not detected, you need to supply Qt5 directories. Below may be useful for Ubuntu/Debian users, although only tested on Ubuntu 12.04.

    ./build.py -I deps/ubuntu.gypi

For more customization, see

    ./build.py --help
