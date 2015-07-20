import QtQuick 2.4
import QtTest 1.1
import Protobuf 1.0 as PB
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

    property string unaryErrorMessage: 'Predefined unary error message'
    property string serverStreamingErrorMessage: 'Predefined server streaming error message'
    property string clientStreamingErrorMessage: 'Predefined client streaming error message'
    property string bidiStreamingErrorMessage: 'Predefined bidi streaming error message'

    // For Qt 5.2.1 (or earlier I guess), we cannot use signals for this
    // because it removes some properties from inccomming messages.
    // So we generate function property to be assigned like this.
    sayHello: function(data, callback) {
      if (data.name === 'GIVE_ME_ERROR') {
        callback(new PB.RpcErrors.Aborted(unaryErrorMessage));
        return;
      }
      callback(null, {
        greet: 'Hello ' + data.name,
      });
    }

    batchHello: function(call, callback) {
      var msg = 'Hello';
      var err = null;
      call.on('data', function(data) {
        if (data.name === 'GIVE_ME_ERROR') {
          err = new PB.RpcErrors.Aborted(clientStreamingErrorMessage);
          return;
        }
        msg += ' ' + data.name;
      });
      call.on('end', function() {
        if (err) {
          callback(err);
          return;
        }
        callback(null, {
          greet: msg,
        });
      });
    }

    subscribeHello: function(call) {
      call.on('data', function(data) {
        for (var i = 0; i < data.requestsSize; ++i) {
          var req = data.requests(i);
          if (req.name === 'GIVE_ME_ERROR') {
            call.error(new PB.RpcErrors.Aborted(serverStreamingErrorMessage));
            return;
          }
          call.write({
            'greet': 'Hello ' + req.name,
          });
        }
        call.end();
      });
    }

    bidiHello: function(call) {
      var queue = [];
      call.on('data', function(data) {
        for (var i = 0; i < data.requestsSize; ++i) {
          if (data.requests(i).name === 'GIVE_ME_ERROR') {
            call.error(new PB.RpcErrors.Aborted(bidiStreamingErrorMessage));
            return;
          }
          queue.push({
            'greet': 'Hello ' + data.requests(i).name,
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

    // same tests as client side test
    property var data: ({ client: helloClient, })
    function test_unary() { Common.test_unary(test, test.data); }
    function test_client_streaming() { Common.test_client_streaming(test, test.data); }
    function test_server_streaming() { Common.test_server_streaming(test, test.data); }
    function test_bidi_streaming() { Common.test_bidi_streaming(test, test.data); }

    // error handling tests
    function test_unary_error() {
      var called = {};
      helloClient.sayHello({
        name: 'GIVE_ME_ERROR',
      }, function(err, data) {
        verify(err);
        compare(err.message, service.unaryErrorMessage);
        compare(err.code, PB.StatusCode.ABORTED);
        compare(err.name, 'Aborted');
        verify(err instanceof PB.RpcErrors.Aborted);
        called.called = true;
      }, 1000);
      tryCompare(called, 'called', true, 2000);
    }

    function test_client_streaming_error() {
      var called = {};
      var call = helloClient.batchHello(function(err, rsp) {
        verify(err);
        compare(err.message, service.clientStreamingErrorMessage);
        compare(err.code, PB.StatusCode.ABORTED);
        compare(err.name, 'Aborted');
        verify(err instanceof PB.RpcErrors.Aborted);
        called.called = true;
      });

      call.timeout = 1000;

      var ok = call.write({name: 'GIVE_ME_ERROR'});
      test.verify(ok);

      ok = call.end();
      test.verify(ok);

      tryCompare(called, 'called', true, 2000);
    }

    function test_server_streaming_error() {
      var called = {};
      var ok = helloClient.subscribeHello({
        requests: [{
          name: 'GIVE_ME_ERROR',
        }],
      }, function(err, rsp, fin) {
        if (!fin) {
          verify(err);
          compare(err.message, service.serverStreamingErrorMessage);
          compare(err.code, PB.StatusCode.ABORTED);
          compare(err.name, 'Aborted');
          verify(err instanceof PB.RpcErrors.Aborted);
          called.called = true;
        }
      }, 1000);
      test.verify(ok);

      tryCompare(called, 'called', true, 2000);
    }

    function test_bidi_streaming_error() {
      var called = {};
      var call = helloClient.bidiHello(function(err, rsp, fin) {
        if (!fin) {
          verify(err);
          compare(err.message, service.bidiStreamingErrorMessage);
          compare(err.code, PB.StatusCode.ABORTED);
          compare(err.name, 'Aborted');
          verify(err instanceof PB.RpcErrors.Aborted);
          called.called = true;
        }
      });
      call.timeout = 1000;
      call.write({
        requests: [{
          name: 'GIVE_ME_ERROR',
        }],
      });
      call.end();
      tryCompare(called, 'called', true, 2000);
    }
  }
}
