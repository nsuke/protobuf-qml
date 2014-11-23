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
      verify(Gen.Msg1.serialize(buffer.output, {i1: -42}));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.i1, -42);
    }

    function test_camel_case() {
      verify(Gen.Msg1.serialize(buffer.output, {
        i1: 0,
        camelFieldTest1: 80,
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.camelFieldTest1, 80);
    }

    function test_minus_64bit() {
      verify(Gen.Msg1.serialize(buffer.output, {
        i1: -80000000000,
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.i1, -80000000000);
    }

    function test_64bit() {
      verify(Gen.Msg1.serialize(buffer.output, {
        i1: 0,
        camelFieldTest1: 80000000000,
      }));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      compare(msg2.camelFieldTest1, 80000000000);
    }

    function test_write_read_missing() {
      verify(Gen.Msg1.serialize(buffer.output, {i1: -42}));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      verify(typeof msg2.i2 == 'undefined');
    }

    function test_repeated() {
      verify(Gen.Msg1.serialize(buffer.output, {
        i1: 0,
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
  }

  TestCase {
    name: 'SerializeTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_serialize_undefined() {
      verify(!Gen.Msg1.serialize(undefined, {i1: 42}));
    }

    function test_serialize_empty() {
      verify(!Gen.Msg1.serialize(buffer.output, {}));
    }

    function test_serialize_missing_required() {
      // TODO: Does not work on Qt5.0 + Ubuntu 12.04
      skip();
      verify(!Gen.Msg1.serialize(buffer.output, {i2: 42}));
    }

    function test_serialize_size0() {
      buffer.size = 0;
      verify(!Gen.Msg1.serialize(buffer.output, {i1: 42}));
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
      verify(Gen.Msg1.serialize(buffer.output, {i1: -42}));
      buffer.size = 0;
      verify(!Gen.Msg1.parse(buffer.input));
    }
  }
}
