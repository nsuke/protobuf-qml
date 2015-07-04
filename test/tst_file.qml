import QtQuick 2.2
import QtTest 1.0
import Protobuf 1.0 as Protobuf
import 'ProtobufQmlTest.pb.js' as Test1
import 'ProtobufQmlTest2.pb.js' as Test2

Item {
  Protobuf.File {
    id: buffer
    // TODO: get random tmp file in a cross platform manner
    path: '/tmp/__tmp__protobuf__qml__'
  }

  TestCase {
    name: 'PrimitiveFieldTest'

    function init() {
      buffer.clear();
    }

    function test_write_read() {
      var called = {};
      var msg1 = new Test1.Msg1({field1: -42});
      msg1.serializeTo(buffer.output, function(err) {
        verify(!err);
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          console.log(msg2);
          verify(!err);
          compare(msg2.field1(), -42);
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }
}
