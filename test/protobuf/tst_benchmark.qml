import QtQuick 2.5
import QtTest 1.1
import Protobuf 1.0 as Protobuf
import 'ProtobufQmlTest.pb.js' as Test1

Item {
  Protobuf.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'SerializationBenchmark'

    property int num: 400

    function createSampleMessage() {
      var msg1 = new Test1.Msg1({
        field1: 20000000000,
        repeatedField: [
          988888, 888898, 888888, 888888, 888888, 888888, 888888, 888888, 888888,
          898888, 888889, 888888, 888888, 888888, 888888, 888888, 888888, 888888,
          889888, 888888, 888888, 888888, 888888, 888888, 888888, 888888, 888888,
          888988, 888888, 888888, 888888, 888888, 888888, 888888, 888888, 888888,
        ],
        stringField: 'foo Bar',
        repeatedStringField: [
          'aaa0', 'aaa1', 'aaa2', 'aaa3', 'aaa4', 'aaa5', 'aaa6', 'aaa7', 'aaa8', 'aaa9',
          'aaa0', 'aaa1', 'aaa2', 'aaa3', 'aaa4', 'aaa5', 'aaa6', 'aaa7', 'aaa8', 'aaa9',
          'aaa0', 'aaa1', 'aaa2', 'aaa3', 'aaa4', 'aaa5', 'aaa6', 'aaa7', 'aaa8', 'aaa9',
        ],
        repeatedEnumField: [
          Test1.Enum1.ENUM_VALUE_DEFAULT, Test1.Enum1.ENUM_VALUE_FIRST, Test1.Enum1.ENUM_VALUE_SECOND, Test1.Enum1.ENUM_VALUE_THIRD,
          Test1.Enum1.ENUM_VALUE_DEFAULT, Test1.Enum1.ENUM_VALUE_FIRST, Test1.Enum1.ENUM_VALUE_SECOND, Test1.Enum1.ENUM_VALUE_THIRD,
        ],
      });
      return msg1;
    }

    function benchmark_v4() {
      for (var i = 0; i < num; ++i) {
        var msg1 = createSampleMessage();
        var buf = msg1.serialize();
        var msg2 = Test1.Msg1.parse(buf);
      }
    }

    function benchmark_v4cb() {
      var called = {called: 0};
      for (var i = 0; i < num; ++i) {
        var msg1 = createSampleMessage();
        msg1.serialize(function(err, buf) {
          err && console.warn(err);
          Test1.Msg1.parse(buf, function(err, msg2) {
            err && console.warn(err);
            ++called.called;
          });
        });
      }
      tryCompare(called, 'called', num, 500);
    }

    function benchmark_lecagy() {
      var called = {called: 0};
      for (var i = 0; i < num; ++i) {
        var msg1 = createSampleMessage();
        msg1.serializeTo(buffer.output, function(err) {
          err && console.warn(err);
          Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
            err && console.warn(err);
            ++called.called;
          });
        });
      }
      tryCompare(called, 'called', num, 500);
    }
  }
}
