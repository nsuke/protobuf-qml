import QtQuick 2.2
import QtTest 1.0
import Grpc 1.0 as G
import Protobuf 1.0 as PB
import 'hello.pb.js' as Hello

Item {
  // Client settings
  G.GrpcChannel {
    id: channel
    target: 'localhost:41387'
    credentials: G.InsecureCredentials {}
  }

  HelloClient {
    id: helloClient
    channel: channel
  }

  // Server settings
  G.GrpcServer {
    id: server
    address: '0.0.0.0:41387'
    credentials: G.InsecureServerCredentials {}
  }

  Hello {
    id: service
    server: server
    // For Qt 5.2.1 (or earlier I guess), we cannot use signals for this
    // because it removes some properties from inccomming messages.
    // So we generate function property to be assigned like this.
    sayHello: function(call, callback) {
      callback(null, {
        greet: 'Hello ' + call.name(),
      });
    }
  }

  TestCase {
    name: 'GrpcServerMethodTest'

    function initTestCase() {
      server.start();
    }

    function cleanupTestCase() {
      server.shutdown();
    }

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
      tryCompare(val, 'called', true, 2000);
    }

    function test_client_streaming() {
      skip('Not implemented');
      var val = {};
      // when start calling service
      var call = helloClient.batchHello(function(rsp, err) {
        // should not receive errror
        verify(!err);

        // should receive response processed with all messages by server
        compare(rsp.greet(), 'Hello Bar1 Bar2 Bar3');
        val.called = true;
      });

      call.timeout = 500;

      // and send 3 messages and not end
      var ok = call.write({name: 'Bar1'});
      verify(ok);
      ok = call.write({name: 'Bar2'});
      verify(ok);
      ok = call.write({name: 'Bar3'});
      verify(ok);

      // should not receive response yet
      verify(!val.called);

      // when notified wirtes done
      ok = call.writeEnd(1000);

      // should receive response
      tryCompare(val, 'called', true);
    }
  }
}
