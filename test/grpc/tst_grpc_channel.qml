// To run this test, you need to run test-hello-async-server program beforehand

import QtQuick 2.2
import QtTest 1.0
import Grpc 1.0 as G
import 'hello.pb.js' as Hello

import 'common.js'as Common

Item {
  // Client settings
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
    id: test
    name: 'GrpcMethodTest'

    property var data: ({ client: helloClient, })
    function test_unary() { Common.test_unary(test, test.data); }
    function test_client_streaming() { Common.test_client_streaming(test, test.data); }
    function test_server_streaming() { Common.test_server_streaming(test, test.data); }
    function test_bidi_streaming() { Common.test_bidi_streaming(test, test.data); }
  }
}
