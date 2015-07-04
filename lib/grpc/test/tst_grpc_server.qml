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
    sayHello: function(data, callback) {
      callback(null, {
        greet: 'Hello ' + data.name(),
      });
    }

    batchHello: function(call, callback) {
      var msg = 'Hello';
      call.on('data', function(data) {
        msg += ' ' + data.name();
      });
      call.on('end', function() {
        callback(null, callback(null, {
          greet: msg,
        }));
      });
    }

    subscribeHello: function(call) {
      call.on('data', function(data) {
        for (var i = 0; i < data.requestsCount(); ++i) {
          var req = data.requests(i);
          call.write({
            'greet': 'Hello ' + req.name(),
          });
        }
        call.end();
      });
    }
  }

  TestCase {
    name: 'GrpcServerMethodTest'

    function initTestCase() {
      skip('Wait for upstream change for graceful shutdown of server.');
      server.start();
    }

    function cleanupTestCase() {
      server.shutdown();
    }

    function test_unary() {
      skip('Wait for upstream change for graceful shutdown of server.');
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
      skip('Wait for upstream change for graceful shutdown of server.');
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

    function test_server_streaming() {
      skip('Wait for upstream change for graceful shutdown of server.');
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
