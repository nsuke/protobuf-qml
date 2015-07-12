import QtQuick 2.4
import QtTest 1.1
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
      var msg1 = new Test2.NestingMessage({
        nestedMessage: {
          nestedField: 'nesting',
        },
      });
      var called = {};
      msg1.serializeTo(buffer.output, function() {
        Test2.NestingMessage.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          verify(msg2.nestedMessage());
          compare(msg2.nestedMessage().nestedField(), 'nesting');
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_deeply_nested() {
      var msg1 = new Test2.NestingMessage({
        evenMoreNestedMessage2: {
          deeplyNestedField: 'nesting message',
        },
      });
      var called = {};
      msg1.serializeTo(buffer.output, function() {
        Test2.NestingMessage.parseFrom(buffer.input, function(err, msg2) {
          verify(msg2);
          verify(msg2.evenMoreNestedMessage2());
          compare(msg2.evenMoreNestedMessage2().deeplyNestedField(), 'nesting message');
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }
  }
}
