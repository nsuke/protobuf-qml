Install
================================================================================

Note for Windows
--------------------------------------------------------------------------------

This library uses C++11 features only available for Visual C++ 2015 or later.

Since there's no Qt binary release for Visual C++ 2015 yet, you'll need to either
wait for Qt 5.6.0 or build Qt yourself. Also, MinGW build might work.


Install dependencies
--------------------------------------------------------------------------------

#### Libraries

* Qt 5.5.0 or later
* Protocol Buffers 3.0 beta-1 or later (Automatically built)
* gRPC 0.11.0 or later (Automatically built)
* zlib (For Windows, automatically built)

#### Build dependencies

* cmake
* ninja
* python
* PyYAML (Python module)
* mako (Python module)
* perl
* go

For example, if you are on Ubuntu 14.04, installation looks like following:

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


Build
--------------------------------------------------------------------------------

    $ ./tools/build_dependencies.py --shared
    $ ./tools/bootstrap.py
    $ source ./tools/setup_env.sh
    $ ninja -C out/Release

or for Windows

    % python tools\build_dependencies.py
    % python tools\bootstrap.py --qt5dir=C:\path\to\Qt\lib\cmake
    % tools\setup_env.bat
    % cd out\Release
    % nmake

Please refer [.travis.yml](.travis.yml) for Linux and [appveyor.yml](appveyor.yml) for Windows too.

Use
--------------------------------------------------------------------------------

Basically there are two components: compiler and library.

### Dependencies

You need to make dependencies available to use any of the components.

There's convenience script that setups environment variables for your current shell session.

    $ source ./tools/setup_env.sh

or for Windows

    % tools\setup_env.bat    

### Compiler

Using protobuf-qml compiler plugin with protobuf compiler, you can generate QML/JS files from your `.proto` files.

    $ protoc --plugin=protoc-gen-qml=out/Release/bin/protoc-gen-qml --qml_out <some_dir> <your_idl>.proto

For Windows, put `.exe` where appropriate:

    % protoc.exe --plugin=protoc-gen-qml=out/Release/bin/protoc-gen-qml.exe --qml_out <some_dir> <your_idl>.proto

Note that `protoc` is supposed to be made available by setup_env.sh(.bat) from previous section.

### Library

Using protobuf-qml library, you can use generated code from previous section in your application.

Just put `out/Release/bin/Protobuf` and `out/Release/bin/Grpc` directories to the same directory
as your executable.

    --- your_qml_app_exe
     |
     -- Protobuf/
     |
     -- Grpc/

It is also good idea to put any `.so` (`.dll` for Windows) files from following directories  there, so that your app can without needing to adjust environment variable every time.

* out/Release/*.so
* build/deps/lib/*.so

or for Windows

* out\Release\*.dll
* build\deps\lib\*.dll
* build\deps\bin\*.dll

On Unix you may still need to add an environment variable:

    LD_LIBRARY_PATH=<your_qml_app's dir>
