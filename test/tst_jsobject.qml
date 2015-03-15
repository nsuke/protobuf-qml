import QtQuick 2.0
import QtTest 1.0
import Protobuf 1.0 as Protobuf
import 'ProtobufQmlTest.pb.js' as Test1
import 'ProtobufQmlTest2.pb.js' as Test2

Item {
  Protobuf.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'JSObjectTest'

    function shouldThrow(f) {
      try {
        f();
        qtest_fail('exception was not thrown', 1);
      } catch(err) {
        console.log('[Expected Exception]: ' + err);
      }
    }

    function test_storage_is_isolated() {
      var msg1 = new Test1.Msg1();
      var msg2 = new Test1.Msg1({field1: 42});
      msg1.field1(4200);
      compare(msg2.field1(), 42);
    }

    function test_valid_field_get() {
      var msg1 = new Test1.Msg1();
      var f1 = msg1.field1();
    }

    function test_invalid_field_get() {
      var msg1 = new Test1.Msg1();
      shouldThrow(function() {
        var f1 = msg1.nonExistent();
      });
    }

    function test_valid_field_set() {
      var msg1 = new Test1.Msg1();
      msg1.field1(42);
    }

    function test_invalid_field_set() {
      var msg1 = new Test1.Msg1();
      shouldThrow(function() {
        msg1.nonExistent(42);
      });
    }

    function test_valid_field_init() {
      var msg1 = new Test1.Msg1({
        field1: 42,
      });
    }

    function test_invalid_field_init() {
      shouldThrow(function() {
        var msg1 = new Test1.Msg1({
          nonExistent: 42,
        });
      });
    }
  }
}
