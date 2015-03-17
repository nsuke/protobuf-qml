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
      skip("TODO: Restore oneof implementation");
      var called = {};
      var msg1 = new Test2.ThirdMessage({
        subMessage: {
          str: 'foo',
        },
      });

      msg1.name('should be erased by the other');
      msg1.subMessage().str('foobar');

      msg1.serializeTo(buffer.output, function() {
        Test2.ThirdMessage.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          verify(msg2.subMessage());
          compare(msg2.name(), '');
          compare(msg2.subMessage().str(), 'foobar');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }
}
