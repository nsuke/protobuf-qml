import QtQuick 2.0
import QtTest 1.0
import Protobuf 1.0 as Qpb
import 'QpbTest.pb.js' as Gen
import 'QpbTest2.pb.js' as Gen2

Item {
  Qpb.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'ReadWriteTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_write_read() {
      verify(Gen.Msg1.serialize(buffer.output, {required1: -42}));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.required1, -42);
    }

    function test_camel_case() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: 0,
        camelFieldTest1: 80,
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.camelFieldTest1, 80);
    }

    function test_multi_message() {
      verify(Gen2.SecondMessage.serialize(buffer.output, {
        str: 'some text',
      }));
      var msg2 = Gen2.SecondMessage.parse(buffer.input);
      verify(msg2);
      compare(msg2.str, 'some text');
    }

    function test_minus_64bit() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: -80000000000,
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.required1, -80000000000);
    }

    function test_64bit() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: 0,
        camelFieldTest1: 80000000000,
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.camelFieldTest1, 80000000000);
    }

    function test_string() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: 0,
        stringField: 'foo Bar',
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.stringField, 'foo Bar');
    }

    function test_repeated_string() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: 0,
        repeatedStringField: [
          'foo Bar',
        ],
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      verify(msg2.repeatedStringField);
      compare(msg2.repeatedStringField.length, 1);
      compare(msg2.repeatedStringField[0], 'foo Bar');
    }

    function test_write_read_missing() {
      verify(Gen.Msg1.serialize(buffer.output, {required1: -42}));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(typeof msg2.optionalField1, 'undefined');
    }

    function test_repeated() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: 0,
        repeatedField: [
          42,
          -42,
          43,
        ],
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      verify(msg2.repeatedField);
      compare(msg2.repeatedField.length, 3);
      compare(msg2.repeatedField[0], 42);
      compare(msg2.repeatedField[1], -42);
      compare(msg2.repeatedField[2], 43);
    }

    function test_enum() {
      compare(Gen.Enum1.ENUM_VALUE_FIRST, 19);
      compare(Gen.Enum1.ENUM_VALUE_SECOND, -38);
      compare(Gen.Enum1.ENUM_VALUE_THIRD, 39284727);
      compare(Gen.Enum1.toString(Gen.Enum1.ENUM_VALUE_FIRST), 'ENUM_VALUE_FIRST');
      compare(Gen.Enum1.toString(Gen.Enum1.ENUM_VALUE_SECOND), 'ENUM_VALUE_SECOND');
      compare(Gen.Enum1.toString(Gen.Enum1.ENUM_VALUE_THIRD), 'ENUM_VALUE_THIRD');
    }

    function test_enum_io() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: 0,
        enumField: Gen.Enum1.ENUM_VALUE_SECOND,
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.enumField, Gen.Enum1.ENUM_VALUE_SECOND);
    }

    function test_repeated_enum_io() {
      verify(Gen.Msg1.serialize(buffer.output, {
        required1: 0,
        repeatedEnumField: [
          Gen.Enum1.ENUM_VALUE_SECOND,
          Gen.Enum1.ENUM_VALUE_THIRD,
          Gen.Enum1.ENUM_VALUE_SECOND,
          Gen.Enum1.ENUM_VALUE_FIRST,
        ],
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      verify(msg2.repeatedEnumField);
      compare(msg2.repeatedEnumField.length, 4);
      compare(Gen.Enum1.toString(msg2.repeatedEnumField[0]), 'ENUM_VALUE_SECOND');
      compare(Gen.Enum1.toString(msg2.repeatedEnumField[1]), 'ENUM_VALUE_THIRD');
      compare(Gen.Enum1.toString(msg2.repeatedEnumField[2]), 'ENUM_VALUE_SECOND');
      compare(Gen.Enum1.toString(msg2.repeatedEnumField[3]), 'ENUM_VALUE_FIRST');
    }

    function test_sub_message() {
      verify(Gen2.Msg2.serialize(buffer.output, {
        msg1: {
          required1: 17,
        },
      }));
      var msg2 = Gen2.Msg2.parse(buffer.input);
      verify(msg2);
      verify(msg2.msg1);
      compare(msg2.msg1.required1, 17);
    }

    function test_repeated_sub_message() {
      verify(Gen2.Msg2.serialize(buffer.output, {
        msgs1: [
          {
            required1: 300,
          },
          {
            required1: 0,
            repeatedStringField: [
              'baz',
            ],
          },
        ],
      }));
      var msg2 = Gen2.Msg2.parse(buffer.input);
      verify(msg2);
      verify(msg2.msgs1);
      compare(msg2.msgs1.length, 2);
      compare(msg2.msgs1[0].required1, 300);
      compare(typeof msg2.msgs1[0].repeatedStringField, 'undefined');
      verify(typeof msg2.msgs1[1].repeatedStringField);
      verify(typeof msg2.msgs1[1].repeatedStringField[0], 'baz');
    }
  }

  TestCase {
    name: 'SerializeTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_serialize_undefined() {
      verify(!Gen.Msg1.serialize(undefined, {required1: 42}));
    }

    function test_serialize_empty() {
      verify(!Gen.Msg1.serialize(buffer.output, {}));
    }

    function test_serialize_missing_required() {
      // TODO: Does not work on Qt5.0 + Ubuntu 12.04
      skip();
      verify(!Gen.Msg1.serialize(buffer.output, {optionalField1: 42}));
    }

    function test_serialize_size0() {
      buffer.size = 0;
      verify(!Gen.Msg1.serialize(buffer.output, {required1: 42}));
    }

    function test_serialize_undefined_value() {
      verify(!Gen.Msg1.serialize(buffer.output, undefined));
    }

    function test_serialize_invalid_value() {
      verify(!Gen.Msg1.serialize(buffer.output, 'invalid'));
    }
  }

  TestCase {
    name: 'ParseTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_parse_undefined() {
      verify(!Gen.Msg1.parse(undefined));
    }

    function test_parse_empty() {
      verify(!Gen.Msg1.parse(buffer.input));
    }

    function test_parse_size0() {
      verify(Gen.Msg1.serialize(buffer.output, {required1: -42}));
      buffer.size = 0;
      verify(!Gen.Msg1.parse(buffer.input));
    }
  }
}
