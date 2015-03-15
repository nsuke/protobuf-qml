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
    name: 'NestingTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_nested() {
      var called = {};
      var msg1 = new Test2.NestingMessage({
        nestedMessage: {
          nestedField: 'nesting',
        },
      });
      msg1.serializeTo(buffer.output, function() {
        Test2.NestingMessage.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          verify(msg2.nestedMessage());
          compare(msg2.nestedMessage().nestedField(), 'nesting');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_deeply_nested() {
      var called = {};
      var msg1 = new Test2.NestingMessage({
        nestedOneof: {
          evenMoreNestedMessage2: {
            deeplyNestedField: 'nesting message',
          },
        },
      });
      msg1.serializeTo(buffer.output, function() {
        Test2.NestingMessage.parseFrom(buffer.input, function(msg2) {
          verify(msg2);
          verify(msg2.nestedOneof());
          verify(msg2.nestedOneof().evenMoreNestedMessage2());
          compare(msg2.nestedOneof().evenMoreNestedMessage2().deeplyNestedField(), 'nesting message');
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }
}
