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
    name: 'PrimitiveFieldTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_write_read() {
      var serialized = {};
      var msg1 = new Test1.Msg1({field1: -42});
      msg1.serializeTo(buffer.output, function() {
        serialized.value = true;
        serialized.set = true;
      }, function(err) {
        serialized.value = false;
        serialized.set = true;
        verify(false);
      });

      tryCompare(serialized, 'set', true);
      verify(serialized.value);

      var parsed = {};
      Test1.Msg1.parseFrom(buffer.input, function(msg) {
        parsed.value = msg;
        parsed.set = true;
      }, function(err) {
        parsed.value = undefined;
        parsed.set = true;
      });
      tryCompare(parsed, 'set', true);
      verify(parsed.value);
      compare(parsed.value.field1, -42);
    }

    function test_camel_case() {
      var msg1 = new Test1.Msg1({
        field1: 0,
        camelFieldTest1: 80,
      });
      var called = {};
      msg1.serializeTo(buffer.output, function() {
        called['value'] = true;
      });
      tryCompare(called, 'value', true, 100);

      var msg2 = null;
      called.value = false;
      Test1.Msg1.parseFrom(buffer.input, function(msg) {
        msg2 = msg;
        called['value'] = true;
      });
      tryCompare(called, 'value', true, 100);
      verify(msg2);
      compare(msg2.camelFieldTest1, 80);
    }

    function test_multi_message() {
      var called = {};
      var msg2 = new Test2.SecondMessage({
        str: 'some text',
      });
      msg2.serializeTo(buffer.output, function() {
        called.serialized = true;
        Test2.SecondMessage.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          compare(msg2.str, 'some text');
          called.parsed = true;
        }, function(err) {
          verify(false);
        });
      }, function(err) {
        verify(false);
      });
      tryCompare(called, 'serialized', true);
      tryCompare(called, 'parsed', true);
    }

    function test_minus_64bit() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: -80000000000,
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          compare(msg2.field1, -80000000000);
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_64bit() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 0,
        camelFieldTest1: 80000000000,
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          compare(msg2.camelFieldTest1, 80000000000);
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_string() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 0,
        stringField: 'foo Bar',
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          compare(msg2.stringField, 'foo Bar');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_write_read_missing() {
      var called = {};
      var msg1 = new Test1.Msg1({field1: -42});
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          compare(typeof msg2.optionalField1, 'undefined');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }

  /*
  TestCase {
    name: 'AsyncTest'

    function init() {
      var called = {};
      buffer.clear();
      buffer.size = 1000;
      tryCompare(called, 'value', true, 100);
    }

    function test_serialize_fail() {
      var called = [false];
      var msg1 = new Test1.Msg1({}, function(err {
        msg1.serializeTo(buffer.output, function() {
          verify(err);
          called[0] = true;
        });
      }));
      tryCompare(called, 0, true, 200);
    }

    function test_parse_fail() {
      var called = [false];
      verify(Test1.Msg1.parseFrom(buffer.input, function(msg, err) {
        verify(err);
        called[0] = true;
      }));
      tryCompare(called, 0, true, 200);
    }

    function test_io() {
      var called = [false, false];
      var msg1 = new Test1.Msg1({field1: -42}, function(err {
        msg1.serializeTo(buffer.output, function() {
          verify(!err);
          called[0] = true;
        });
      }));
      tryCompare(called, 0, true, 200);
      verify(Test1.Msg1.parseFrom(buffer.input, function(msg, err) {
        compare(msg.field1, -42);
        verify(!err);
        called[1] = true;
      }));
      tryCompare(called, 1, true, 200);
    }
  }
  */
}
