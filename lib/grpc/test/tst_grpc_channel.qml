import QtQuick 2.2
import QtTest 1.0
import Grpc 1.0 as G
import Protobuf 1.0 as P
import 'hello.pb.js' as Hello

Item {
  G.GrpcChannel {
    id: channel
    target: 'localhost:41384'
    credentials: G.InsecureCredentials {}
  }

  // TODO: Implement code generator for this
  Item {
    id: helloClient

    P.UnaryMethod {
      id: sayHelloMethod
      channel: channel
      methodName: '/Hello/SayHello'
      readDescriptor: Hello.HelloResponse.descriptor
      writeDescriptor: Hello.HelloRequest.descriptor
    }

    function sayHello(data, callback, timeout) {
      return sayHelloMethod.call(
        new Hello.HelloRequest(data)._raw,
        function(data, err) {
          callback(new Hello.HelloResponse(data), err);
        },
        timeout);
    }
  }

  TestCase {
    name: 'GrpcUnaryMethodTest'

    function test_hello() {
      var val = {};
      var ok = helloClient.sayHello({name: 'Foo'}, function(rsp) {
        compare(rsp.greet(), 'Hello Foo');
        val.called = true;
      }, 500);
      verify(ok);
      tryCompare(val, 'called', true, 600);
    }
  }
}
