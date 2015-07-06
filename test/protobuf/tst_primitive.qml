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
    name: 'PrimitiveFieldTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_write_read() {
      var called = {};
      var msg1 = new Test1.Msg1({field1: -42});
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          compare(msg2.field1(), -42);
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_camel_case() {
      var msg1 = new Test1.Msg1({
        field1: 0,
        camelFieldTest1: 80,
      });
      var called = {};
      msg1.serializeTo(buffer.output, function() {
        called.called = true;
      });
      tryCompare(called, 'called', true, 100);

      var msg2 = null;
      called.called = false;
      Test1.Msg1.parseFrom(buffer.input, function(err, msg) {
        msg2 = msg;
        called.called = true;
      });
      tryCompare(called, 'called', true, 100);
      verify(msg2);
      compare(msg2.camelFieldTest1(), 80);
    }

    function test_multi_message() {
      var called = {};
      var msg2 = new Test2.SecondMessage({
        str: 'some text',
      });
      msg2.serializeTo(buffer.output, function(err) {
        if (err) {
          fail('serialize error');
        }
        called.serialized = true;
        Test2.SecondMessage.parseFrom(buffer.input, function(err, msg2) {
          if (err) {
            fail('parse error');
          }
          verify(msg2);
          compare(msg2.str(), 'some text');
          called.parsed = true;
        });
      });
      tryCompare(called, 'serialized', true, 100);
      tryCompare(called, 'parsed', true, 100);
    }

    function test_minus_64bit() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: -80000000000,
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          compare(msg2.field1(), -80000000000);
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_64bit() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 0,
        camelFieldTest1: 80000000000,
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          compare(msg2.camelFieldTest1(), 80000000000);
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_string() {
      var called = {};
      var msg1 = new Test1.Msg1({
        field1: 9,
        stringField: 'foo Bar',
      });
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          compare(msg2.stringField(), 'foo Bar');
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_write_read_missing() {
      skip('TODO: default value test');
      var called = {};
      var msg1 = new Test1.Msg1({field1: -42});
      msg1.serializeTo(buffer.output, function() {
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          compare(typeof msg2.optionalField1(), 'undefined');
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }
  }
}
