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
    name: 'RepeatedFieldTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_repeated_append() {
      var msg1 = new Test1.Msg1();
      msg1.addRepeatedField(42);
      msg1.addRepeatedField(100);
      msg1.addRepeatedField(-99);
      compare(msg1.repeatedFieldSize, 3);
      compare(msg1.repeatedField(0), 42);
      compare(msg1.repeatedField(1), 100);
      compare(msg1.repeatedField(2), -99);
    }

    function test_repeated_setter() {
      var msg1 = new Test1.Msg1();
      msg1.addRepeatedField(42);
      msg1.addRepeatedField(100);
      msg1.addRepeatedField(-99);
      msg1.repeatedField(1, 33);
      compare(msg1.repeatedField(1), 33);
    }

    function test_repeated_setter_out_of_range() {
      var msg1 = new Test1.Msg1();
      try {
        msg1.repeatedField(1, 42);
        fail('Out of range setter should throw');
      } catch (err) {
      }
    }

    function test_repeated() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 0,
        repeatedField: [
          42,
          -42,
          43,
        ],
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          verify(msg2.repeatedField);
          compare(msg2.repeatedFieldLength, 3);
          compare(msg2.repeatedFieldCount, 3);
          compare(msg2.repeatedFieldSize, 3);
          compare(msg2.getRepeatedFieldLength(), 3);
          compare(msg2.getRepeatedFieldCount(), 3);
          compare(msg2.getRepeatedFieldSize(), 3);
          compare(msg2.repeatedField(0), 42);
          compare(msg2.repeatedField(1), -42);
          compare(msg2.repeatedField(2), 43);
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_repeated_string() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 0,
        repeatedStringField: [
          'foo Bar',
          'FooBar 2',
        ],
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          verify(msg2.repeatedStringField);
          compare(msg2.repeatedStringFieldSize, 2);
          compare(msg2.repeatedStringField(0), 'foo Bar');
          compare(msg2.repeatedStringField(1), 'FooBar 2');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_repeated_enum_io() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 0,
        repeatedEnumField: [
          Test1.Enum1.ENUM_VALUE_SECOND,
          Test1.Enum1.ENUM_VALUE_THIRD,
          Test1.Enum1.ENUM_VALUE_SECOND,
          Test1.Enum1.ENUM_VALUE_FIRST,
        ],
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          verify(msg2.repeatedEnumField);
          compare(msg2.repeatedEnumFieldCount, 4);
          compare(Test1.Enum1.toString(msg2.repeatedEnumField(0)), 'ENUM_VALUE_SECOND');
          compare(Test1.Enum1.toString(msg2.repeatedEnumField(1)), 'ENUM_VALUE_THIRD');
          compare(Test1.Enum1.toString(msg2.repeatedEnumField(2)), 'ENUM_VALUE_SECOND');
          compare(Test1.Enum1.toString(msg2.repeatedEnumField(3)), 'ENUM_VALUE_FIRST');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_repeated_sub_message() {
      var called = {};
      var msg1 = new Test2.Msg2({
        msgs1: [
          {
            field1: 300,
          },
          {
            field1: 0,
            repeatedStringField: [
              'baz',
            ],
          },
        ],
      });
      msg1.serializeTo(buffer.output, function() {
        Test2.Msg2.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          verify(msg2.msgs1);
          compare(msg2.msgs1Count, 2);
          compare(msg2.msgs1(0).field1, 300);
          compare(msg2.msgs1(0).repeatedStringFieldCount, 0);
          verify(msg2.msgs1(1).repeatedStringFieldCount > 0);
          verify(typeof msg2.msgs1(1).repeatedStringField(0), 'baz');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }
}
