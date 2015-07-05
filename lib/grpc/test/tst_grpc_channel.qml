// To run this test, you need to run hello-server program beforehand

import QtQuick 2.2
import QtTest 1.0
import Grpc 1.0 as G
import 'hello.pb.js' as Hello

Item {
  G.GrpcChannel {
    id: channel
    target: 'localhost:41384'
    credentials: G.InsecureCredentials {}
  }

  HelloClient {
    id: helloClient
    channel: channel
  }

  TestCase {
    name: 'GrpcMethodTest'

    function test_unary() {
      var val = {};
      // when invoked service
      var ok = helloClient.sayHello({name: 'Foo'}, function(err, rsp) {
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
      var call = helloClient.batchHello(function(err, rsp) {
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
      ok = call.end(1000);

      // should receive response
      tryCompare(val, 'called', true);
    }

    function test_server_streaming() {
      var end = {};
      var received = [];

      var ok = helloClient.subscribeHello({
        requests: [{
          name: 'Baz0',
        } , {
          name: 'Baz1',
        } , {
          name: 'Baz2',
        } , {
          name: 'Baz3',
        }],
      }, function(err, data, finished) {
        verify(!err);
        if (finished) {
          end.called = true;
        } else {
          received.push(data.greet());
        }
      });
      verify(ok);

      tryCompare(end, 'called', true);
      compare(received.length, 4);
      compare(received[0], 'Hello Baz0');
      compare(received[1], 'Hello Baz1');
      compare(received[2], 'Hello Baz2');
      compare(received[3], 'Hello Baz3');
    }
  }
}
