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

    P.WriterMethod {
      id: batchHelloMethod
      channel: channel
      methodName: '/Hello/BatchHello'
      readDescriptor: Hello.HelloResponse.descriptor
      writeDescriptor: Hello.HelloRequest.descriptor
    }

    function batchHello(callback) {
      var call = batchHelloMethod.call(
        function(data, err) {
          callback(new Hello.HelloResponse(data), err);
        });
      return {
        write: function(data, timeout) {
          return call.write(new Hello.HelloRequest(data)._raw, timeout);
        },
        writeEnd: call.writeEnd,
      };
    }
  }

  TestCase {
    name: 'GrpcMethodTest'

    function test_unary() {
      var val = {};
      // when invoked service
      var ok = helloClient.sayHello({name: 'Foo'}, function(rsp, err) {
        // should not receive errror
        verify(!err);

        // should receive response processed by server
        compare(rsp.greet(), 'Hello Foo');
        val.called = true;
      }, 500);

      // should be able to send
      verify(ok);

      // should receive response
      tryCompare(val, 'called', true, 600);
    }

    function test_client_streaming() {
      var val = {};
      // when start calling service
      var call = helloClient.batchHello(function(rsp, err) {
        // should not receive errror
        verify(!err);

        // should receive response processed with all messages by server
        compare(rsp.greet(), 'Hello Bar1 Bar2 Bar3');
        val.called = true;
      });

      // and send 3 messages and not end
      var ok = call.write({name: 'Bar1'}, 1000);
      // TODO: why the wait needed ?
      wait(400);
      verify(ok);
      ok = call.write({name: 'Bar2'}, 1000);
      verify(ok);
      ok = call.write({name: 'Bar3'}, 1000);
      verify(ok);

      // should not receive response yet
      verify(!val.called);

      // when notified wirtes done
      ok = call.writeEnd(1000);

      // should receive response
      tryCompare(val, 'called', true, 2000);
    }
  }
}
