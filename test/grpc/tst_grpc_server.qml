import QtQuick 2.2
import QtTest 1.0
import Grpc 1.0 as G
import 'hello.pb.js' as Hello

import 'common.js'as Common

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

    bidiHello: function(call) {
      var queue = [];
      call.on('data', function(data) {
        for (var i = 0; i < data.requestsCount(); ++i) {
          queue.push({
            'greet': 'Hello ' + data.requests(i).name(),
          });
        }
      });
      call.on('end', function(data) {
        for (var i in queue) {
          call.write(queue[i]);
        }
        call.end();
      });
    }
  }

  TestCase {
    id: test
    name: 'GrpcServerMethodTest'

    function initTestCase() {
      server.start();
      wait(50);
    }

    function cleanupTestCase() {
      server.shutdown();
    }

    property var data: ({ client: helloClient, })
    function test_unary() { Common.test_unary(test, test.data); }
    function test_client_streaming() { Common.test_client_streaming(test, test.data); }
    function test_server_streaming() { Common.test_server_streaming(test, test.data); }
    function test_bidi_streaming() { Common.test_bidi_streaming(test, test.data); }
  }
}
