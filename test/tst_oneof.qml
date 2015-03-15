import QtQuick 2.2
import QtTest 1.0
import Protobuf 1.0 as Protobuf
import 'ProtobufQmlTest.pb.js' as Test1
import 'ProtobufQmlTest2.pb.js' as Test2

Item {
  Protobuf.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'OneofFieldTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_oneof() {
      var called = {};
      var msg1 = new Test2.ThirdMessage({
        testOneof: {
          subMessage: {
            str: 'foobar',
          },
        },
      });
      msg1.serializeTo(buffer.output, function() {
        Test2.ThirdMessage.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          verify(msg2.testOneof);
          verify(msg2.testOneof.subMessage);
          compare(typeof msg2.testOneof.name, 'undefined');
          compare(msg2.testOneof.subMessage.str, 'foobar');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }
}
