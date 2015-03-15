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
    name: 'MessageFieldTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_enum_io() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 0,
        enumField: Test1.Enum1.ENUM_VALUE_SECOND,
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          compare(msg2.enumField(), Test1.Enum1.ENUM_VALUE_SECOND);
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_sub_message() {
      var called = {};
      var msg1 = new Test2.Msg2({
        msg1: {
          field1: 17,
        },
      });
      verify(msg1.msg1());
      compare(msg1.msg1().field1(), 17);
      msg1.serializeTo(buffer.output, function() {
        Test2.Msg2.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          verify(msg2.msg1());
          compare(msg2.msg1().field1(), 17);
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }
}
