import QtQuick 2.5
import QtTest 1.1
import Protobuf 1.0 as Protobuf
import 'ProtobufQmlTest.pb.js' as Test1
import 'ProtobufQmlTest2.pb.js' as Test2

Item {
  Protobuf.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'JSObjectTest'

    function shouldThrow(f, keyword) {
      try {
        f();
        qtest_fail('exception was not thrown', 1);
      } catch(err) {
        console.log('[Expected Exception]: ' + err);
        if (keyword && (String(err).indexOf(keyword) < 0)) {
          qtest_fail('Expected keyword [' + keyword
              + '] was not found in error message [' + String(err) + ']', 2);
        }
      }
    }

    function test_storage_is_isolated() {
      var msg1 = new Test1.Msg1();
      var msg2 = new Test1.Msg1({field1: 42});
      msg1.field1 = 4200;
      compare(msg2.field1, 42);
    }

    function test_valid_field_get() {
      var msg1 = new Test1.Msg1();
      var f1 = msg1.field1;
      var f2 = msg1.getField1();
      // no error
    }

    function test_invalid_field_get() {
      var msg1 = new Test1.Msg1();
      // Function access should throw concise message.
      shouldThrow(function() {
        var f1 = msg1.getNonExistent();
      }, 'NonExistent');

      // Unable to make property access to throw.
      var f1 = msg1.nonExistent;
      verify(typeof f1 == 'undefined');
    }

    function test_valid_field_set() {
      var msg1 = new Test1.Msg1();
      msg1.field1 = 42;
      msg1.setField1(42);
      // no error
    }

    function test_invalid_field_set() {
      // Function access should throw concise message.
      var msg1 = new Test1.Msg1();
      shouldThrow(function() {
        msg1.setNonExistent(42);
      }, 'NonExistent');

      // Unable to make property access to throw.
      msg1.nonExistent = 42;
      skip('Disabled Object.seal for performance reason.');
      verify(typeof msg1.nonExistent == 'undefined');
    }

    function test_valid_field_init() {
      var msg1 = new Test1.Msg1({
        field1: 42,
      });
      // no error
    }

    function test_invalid_field_init() {
      skip('Disabled Object.seal for performance reason.');
      shouldThrow(function() {
        var msg1 = new Test1.Msg1({
          nonExistent: 42,
        });
      }, 'nonExistent');
    }
  }
}
