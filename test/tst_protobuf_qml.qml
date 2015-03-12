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
    name: 'ReadWriteTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_write_read() {
      verify(Test1.Msg1.serialize(buffer.output, {field1: -42}));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.field1, -42);
    }

    function test_camel_case() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: 0,
        camelFieldTest1: 80,
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.camelFieldTest1, 80);
    }

    function test_multi_message() {
      verify(Test2.SecondMessage.serialize(buffer.output, {
        str: 'some text',
      }));
      var msg2 = Test2.SecondMessage.parse(buffer.input);
      verify(msg2);
      compare(msg2.str, 'some text');
    }

    function test_minus_64bit() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: -80000000000,
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.field1, -80000000000);
    }

    function test_64bit() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: 0,
        camelFieldTest1: 80000000000,
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.camelFieldTest1, 80000000000);
    }

    function test_string() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: 0,
        stringField: 'foo Bar',
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.stringField, 'foo Bar');
    }

    function test_repeated_string() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: 0,
        repeatedStringField: [
          'foo Bar',
        ],
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      verify(msg2.repeatedStringField);
      compare(msg2.repeatedStringField.length, 1);
      compare(msg2.repeatedStringField[0], 'foo Bar');
    }

    function test_write_read_missing() {
      verify(Test1.Msg1.serialize(buffer.output, {field1: -42}));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      compare(typeof msg2.optionalField1, 'undefined');
    }

    function test_repeated() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: 0,
        repeatedField: [
          42,
          -42,
          43,
        ],
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      verify(msg2.repeatedField);
      compare(msg2.repeatedField.length, 3);
      compare(msg2.repeatedField[0], 42);
      compare(msg2.repeatedField[1], -42);
      compare(msg2.repeatedField[2], 43);
    }

    function test_enum() {
      compare(Test1.Enum1.ENUM_VALUE_FIRST, 19);
      compare(Test1.Enum1.ENUM_VALUE_SECOND, -38);
      compare(Test1.Enum1.ENUM_VALUE_THIRD, 39284727);
      compare(Test1.Enum1.toString(Test1.Enum1.ENUM_VALUE_FIRST), 'ENUM_VALUE_FIRST');
      compare(Test1.Enum1.toString(Test1.Enum1.ENUM_VALUE_SECOND), 'ENUM_VALUE_SECOND');
      compare(Test1.Enum1.toString(Test1.Enum1.ENUM_VALUE_THIRD), 'ENUM_VALUE_THIRD');
    }

    function test_enum_io() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: 0,
        enumField: Test1.Enum1.ENUM_VALUE_SECOND,
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.enumField, Test1.Enum1.ENUM_VALUE_SECOND);
    }

    function test_repeated_enum_io() {
      verify(Test1.Msg1.serialize(buffer.output, {
        field1: 0,
        repeatedEnumField: [
          Test1.Enum1.ENUM_VALUE_SECOND,
          Test1.Enum1.ENUM_VALUE_THIRD,
          Test1.Enum1.ENUM_VALUE_SECOND,
          Test1.Enum1.ENUM_VALUE_FIRST,
        ],
      }));
      var msg2 = Test1.Msg1.parse(buffer.input);
      verify(msg2);
      verify(msg2.repeatedEnumField);
      compare(msg2.repeatedEnumField.length, 4);
      compare(Test1.Enum1.toString(msg2.repeatedEnumField[0]), 'ENUM_VALUE_SECOND');
      compare(Test1.Enum1.toString(msg2.repeatedEnumField[1]), 'ENUM_VALUE_THIRD');
      compare(Test1.Enum1.toString(msg2.repeatedEnumField[2]), 'ENUM_VALUE_SECOND');
      compare(Test1.Enum1.toString(msg2.repeatedEnumField[3]), 'ENUM_VALUE_FIRST');
    }

    function test_oneof() {
      verify(Test2.ThirdMessage.serialize(buffer.output, {
        testOneof: {
          subMessage: {
            str: 'foobar',
          },
        },
      }));
      var msg2 = Test2.ThirdMessage.parse(buffer.input);
      verify(msg2);
      verify(msg2.testOneof);
      verify(msg2.testOneof.subMessage);
      compare(typeof msg2.testOneof.name, 'undefined');
      compare(msg2.testOneof.subMessage.str, 'foobar');
    }

    function test_sub_message() {
      verify(Test2.Msg2.serialize(buffer.output, {
        msg1: {
          field1: 17,
        },
      }));
      var msg2 = Test2.Msg2.parse(buffer.input);
      verify(msg2);
      verify(msg2.msg1);
      compare(msg2.msg1.field1, 17);
    }

    function test_repeated_sub_message() {
      verify(Test2.Msg2.serialize(buffer.output, {
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
      }));
      var msg2 = Test2.Msg2.parse(buffer.input);
      verify(msg2);
      verify(msg2.msgs1);
      compare(msg2.msgs1.length, 2);
      compare(msg2.msgs1[0].field1, 300);
      compare(typeof msg2.msgs1[0].repeatedStringField, 'undefined');
      verify(typeof msg2.msgs1[1].repeatedStringField);
      verify(typeof msg2.msgs1[1].repeatedStringField[0], 'baz');
    }
  }

  TestCase {
    name: 'NestTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_nested() {
      verify(Test2.NestingMessage.serialize(buffer.output, {
        nestedMessage: {
          nestedField: 'nesting',
        },
      }));
      var msg = Test2.NestingMessage.parse(buffer.input);
      verify(msg);
      verify(msg.nestedMessage);
      compare(msg.nestedMessage.nestedField, 'nesting');
    }

    function test_deeply_nested() {
      verify(Test2.NestingMessage.serialize(buffer.output, {
        nestedOneof: {
          evenMoreNestedMessage2: {
            deeplyNestedField: 'nesting message',
          },
        },
      }));
      var msg = Test2.NestingMessage.parse(buffer.input);
      verify(msg);
      verify(msg.nestedOneof);
      verify(msg.nestedOneof.evenMoreNestedMessage2);
      compare(msg.nestedOneof.evenMoreNestedMessage2.deeplyNestedField, 'nesting message');
    }
  }

  TestCase {
    name: 'AsyncTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_serialize_fail() {
      var called = [false];
      verify(Test1.Msg1.serialize(buffer.output, {}, function(err) {
        verify(err);
        called[0] = true;
      }));
      tryCompare(called, 0, true, 200);
    }

    function test_parse_fail() {
      var called = [false];
      verify(Test1.Msg1.parse(buffer.input, function(msg, err) {
        verify(err);
        called[0] = true;
      }));
      tryCompare(called, 0, true, 200);
    }

    function test_io() {
      var called = [false, false];
      verify(Test1.Msg1.serialize(buffer.output, {field1: -42}, function(err) {
        verify(!err);
        called[0] = true;
      }));
      tryCompare(called, 0, true, 200);
      verify(Test1.Msg1.parse(buffer.input, function(msg, err) {
        compare(msg.field1, -42);
        verify(!err);
        called[1] = true;
      }));
      tryCompare(called, 1, true, 200);
    }
  }

  TestCase {
    name: 'SerializeTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_serialize_undefined() {
      verify(!Test1.Msg1.serialize(undefined, {field1: 42}));
    }

    function test_serialize_empty() {
      verify(!Test1.Msg1.serialize(buffer.output, {}));
    }

    function test_serialize_size0() {
      buffer.size = 0;
      verify(!Test1.Msg1.serialize(buffer.output, {field1: 42}));
    }

    function test_serialize_undefined_value() {
      verify(!Test1.Msg1.serialize(buffer.output, undefined));
    }

    function test_serialize_invalid_value() {
      verify(!Test1.Msg1.serialize(buffer.output, 'invalid'));
    }
  }

  TestCase {
    name: 'ParseTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_parse_undefined() {
      verify(!Test1.Msg1.parse(undefined));
    }

    function test_parse_empty() {
      verify(!Test1.Msg1.parse(buffer.input));
    }

    function test_parse_size0() {
      verify(Test1.Msg1.serialize(buffer.output, {field1: -42}));
      buffer.size = 0;
      verify(!Test1.Msg1.parse(buffer.input));
    }
  }
}
