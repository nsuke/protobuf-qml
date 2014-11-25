protobuf-qml
============
Protocol Buffers integration for QML (QtQuick2)
---
[![Build Status](https://travis-ci.org/nsuke/protobuf-qml.svg?branch=master)](https://travis-ci.org/nsuke/protobuf-qml)

Supported platforms
----
Since both Protocol Buffers and Qt are cross platform, soruce code of this library can be compiled for and run on many platforms.
However currently only linux build configuration is available out of the box.

Tested with following library versions:
* Protocol Buffers 2.6.1
* Qt 5.0 and 5.4

Buiding:
---

Following command will download some dependencies, build, and run tests.
You need Qt5 installed on your system.

    ./build.py

If your Qt5 installation is not detected, you need to supply Qt5 directories. Below may be useful for Ubuntu/Debian users, although only tested on Ubuntu 12.04 travis servers.

    ./build.py -I deps/ubuntu.gypi

For more customization, see

    ./build.py --help

TODO:
----
* Support "extension" and unknown fields
* Getting started documentation
* Android build configuration
