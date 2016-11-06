protobuf-qml
============

[![Build Status](https://travis-ci.org/nsuke/protobuf-qml.svg?branch=master)](https://travis-ci.org/nsuke/protobuf-qml)
[![Build status](https://ci.appveyor.com/api/projects/status/3gaaq96kndvdwvno/branch/master?svg=true)](https://ci.appveyor.com/project/nsuke/protobuf-qml)

Efficient schematized serialization and RPC for QtQuick2 applications through Protocol Buffers and gRPC bindings.

## Usage

For installation, see [INSTALL.md](INSTALL.md).

### Protocol Buffers binding

For Protocol Buffers itself, see upstream official site:

https://developers.google.com/protocol-buffers/

#### Code generation
Suppose you write following .proto file, say `my_foo.proto`:

```
    message Foo {
      string text = 1;
      repeated uint32 nums = 2;
    }
```

you can generate QML/Javascript message type using compiler plugin:

    $ protoc --qml_out gen my_foo.proto

This will yield `gen/my_foo.pb.js` file. Let's import this file from any QML using relative path.

##### Docker image

If you want to try it out without building protobuf-qml, a docker image for code generator is available.

    # docker pull nsuke/protobuf-qml
    # docker run -it -v $(pwd):/opt/protobuf-qml nsuke/protobuf-qml -I. \
        --qml_out=<relative path to output dir> \
        <relative path to your .proto file>

Note that the relative paths cannot contain parent or sibling directories, i.e., they need to be within current or sub directories.

#### Serialization

``` javascript
import 'gen/my_foo.pb.js' as Types
```

then you can use the message type inside signal handlers, property bindings or functions:

``` javascript
var foo = new Types.Foo({
  text: 'hello world',
  nums: [ 3, 4, 5 ],
});

var buf = foo.serialize();
```

here, the `buf` variable is an `ArrayBuffer` object.

<!--
You can for example send it to a server using **[XmlHttpRequest](http://doc.qt.io/qt-5/qtqml-javascript-qmlglobalobject.html#xmlhttprequest)** (not yet, wait for Qt 5.7) or pass it to C++ layer. If your use case is remote procedure call, **gRPC** section below might be intersting.
-->

#### Deserialization

Deserialization is quite simple too:

``` javascript
var foo2 = Types.Foo.parse(buf);

console.log(foo2.text)
// output: hello world
console.log(foo2.nums(1))
// output: 4
```

TBD: Link to full sample code

gRPC binding
---

For gRPC itself, see upstream official page: http://www.grpc.io/

gRPC binding is still experimental.

#### Code generation

Suppose you add service definition to the `my_foo.proto` above:

```
service MyService {
  rpc ExchangeFoo(Foo) returns(Foo) {}
}
```

compiler plugin will additionally yield `MyService.qml` and `MyServiceClient.qml` files besides `my_foo.pb.js` file.
Let's import the directory containing those QML files:

```javascript
import 'gen'
```

then you can instantiate QML elements:

```javascript
MyServiceClient {
  id: client
}
MyService {
  id: service
}
```

#### Client

To make the client element functional, plug it to a gRPC channel.
(In fact, you can plug to custom RPC implementation but gRPC works out of the box)

```javascript
import 'gen'
import gRPC
```

```
GrpcChannel {
  id: gchannel
  target: 'example.org:44448'
  credentials: InsecureCredentials {}
}

MyServiceClienit {
  id: client
  channel: gchannel
}
```

Then inside signal handlers, property bindings or functions :

```javascript
client.exchangeFoo({
  text: 'hi',
  nums: [1, 2, 3],
}, function(err, responseFoo) {
  if (err) {
    console.warn('Received error response: ' + err);
    return;
  }

  // Do some useful stuff with "responseFoo" content
});
```

This will make a RPC call to example.org port 44448.

#### Server

You can create gRPC server in QML app if you want to. It's handy for P2P communication and/or notifications, but not suitable for heavy computation.

```javascript
import 'gen'
import gRPC
```

```javascript
GrpcServer {
  id: gserver
  address: '0.0.0.0:44448'
  credentials: ServerInsecureCredentials {}
}

MyService {
  id: server
  server: gserver

  // This function is called for each incoming request.
  exchangeFoo: function(requestFoo, callback) {

    // First argument is error.
    callback(null, {
      text: 'In response to ' + requestFoo.text,
      nums: [42],
    });
  }
}
```

### QtWebSockets with Qt 5.8+

From Qt 5.8 (QtWebSockets 1.1 QML module) on, we can send array buffers through websockets.

#### Sending serialized message with QtWebSockets

``` javascript
import QtWebSockets 1.1
```

``` javascript
WebSocket {
  id: socket
  url: 'ws://your.remote.server'
  active: true
}
```

``` javascript
// Inside function or handler
var msg = new Types.Foo({
  text: 'hello world',
  nums: [ 3, 4, 5 ],
});
var buf = msg.serialize();
socket.sendBinaryMessage(buf);
```

#### Receiving serialized message with QtWebSockets

``` javascript
WebSocket {
  id: socket
  url: 'ws://your.remote.server'
  active: true

  onBinaryMessageReceived: {
    var msg = Types.Foo.parse(message);
    console.log(msg.text);
    console.log(msg.nums(0));
    console.log(msg.nums(1));
    // ... Do whatever with the data
  }
}
```

The complete code is available under [examples/WebSockets](examples/WebSockets) directory.

### Future examples

In the future, it might become possible to send ArrayBuffer using XMLHttpRequest too.

I've submitted a patch for [XMLHttpRequest](https://codereview.qt-project.org/#/c/143732/) to Qt project which enables following usage.

### Send and receive serialized message with XMLHttpRequest

``` javascript
// Inside function or handler
var xhr = new XMLHttpRequest();
xhr.open('POST', 'http://your.remote.server/');
xhr.onreadystatechange = function() {
  if (xhr.readyState === 4 && xhr.status === 200) {
    var response = Types.Foo.parse(xhr.response);
    // ... Do whatever with the data
  }
};
xhr.responseType = 'arraybuffer';

var msg = new Types.Foo({
  text: 'hello world',
  // ...,
});

var buf = msg.serialize();

xhr.send(new DataView(buf));
```

The complete code is available under [examples/XMLHttpRequest](examples/XMLHttpRequest) directory.
