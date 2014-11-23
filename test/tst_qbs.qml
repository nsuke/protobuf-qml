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

    function test_write_read_missing() {
      verify(Gen.Msg1.serialize(buffer.output, {i1: -42}));
      var msg2 = Gen.Msg1.parse(buffer.input);
      verify(msg2);
      verify(typeof msg2.i2 == 'undefined');
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
