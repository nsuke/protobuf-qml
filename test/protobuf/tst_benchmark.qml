import QtQuick 2.5
import QtTest 1.1
import Protobuf 1.0 as Protobuf
import 'serialization_test.pb.js' as Test1

Item {
  id: root
  Protobuf.MemoryBuffer {
    id: buffer
  }

  property int numLoop: 10
  property int numMessage: 500
  property int numPrimitive: 3000

  TestCase {
    name: 'SerializationBenchmark'
    property alias numLoop: root.numLoop
    property alias numMessage: root.numMessage
    property alias numPrimitive: root.numPrimitive

    function createSubMessage(size) {
      if (typeof size == 'undefined') {
        size = numPrimitive;
      }
      var msg = new Test1.Message1();

      msg.setDoubleField1(42.42);
      msg.setFloatField1(42.42);
      msg.setInt32Field1(42);
      msg.setInt64Field1(42);
      msg.setUint32Field1(42);
      msg.setUint64Field1(42);
      msg.setSint32Field1(42);
      msg.setSint64Field1(42);
      msg.setFixed32Field1(42);
      msg.setFixed64Field1(42);
      msg.setSfixed32Field1(-888888888);
      msg.setSfixed64Field1(42);
      msg.setBoolField1(true);
      msg.setStringField1('42');
      msg.setEnumField1(Test1.Enum2.ENUM_VALUE_2);
      msg.setDoubleField2(-42.42);
      msg.setFloatField2(-42.42);
      msg.setInt32Field2(42);
      msg.setInt64Field2(-999930000000042);
      msg.setUint64Field2(999930000000042);
      msg.setSfixed32Field2(-888888888);
      msg.setSfixed64Field2(-999930000000042);
      msg.setEnumField2(Test1.Enum2.ENUM_VALUE_3);
      msg.setDoubleField3([442.42, -2.42, 442.42, -2.42]);
      msg.setFloatField3([442.42, -2.42, 442.42, -2.42]);
      msg.setInt32Field3([442, -2, 442, -2]);
      msg.setInt64Field3([442, -999999999992, 442, -999999999992]);
      msg.setUint32Field3([442, 2, 442, 2]);
      msg.setUint64Field3([442, 999999999992, 442, 999999999992]);
      msg.setSint32Field3([442, -2, 442, -2]);
      msg.setSint64Field3([442, -999999999992, 442, -999999999992]);
      msg.setFixed32Field3([442, 2, 442, 2]);
      msg.setFixed64Field3([442, 999999999992, 442, 999999999992]);
      msg.setSfixed32Field3([442, -2, 442, -2]);
      msg.setSfixed64Field3([442, -999999999992, 442, -999999999992]);
      msg.setBoolField3([true, false, true]);
      msg.setStringField3(['42', 'foo', 'bar']);
      msg.setEnumField3([Test1.Enum2.ENUM_VALUE_2, Test1.Enum2.ENUM_VALUE_4, Test1.Enum2.ENUM_VALUE_3]);

      var doubleField3 = new Float64Array(size);
      var floatField3 = new Float32Array(size);
      var int32Field3 = new Int32Array(size);
      var uint32Field3 = new Uint32Array(size);
      var sint32Field3 = new Int32Array(size);
      var fixed32Field3 = new Uint32Array(size);
      var sfixed32Field3 = new Int32Array(size);

      var int64Field3 = new Array(size);
      var uint64Field3 = new Array(size);
      var sint64Field3 = new Array(size);
      var fixed64Field3 = new Array(size);
      var sfixed64Field3 = new Array(size);
      var boolField3 = new Int8Array(size);
      var stringField3 = new Array(size);
      var enumField3 = new Array(size);

      for (var i = 0; i < size; ++i) {
        doubleField3[i] = 1234134.1341234;
        floatField3[i] = 1234134.1341234;
        int32Field3[i] = 442;
        uint32Field3[i] = 442;
        int64Field3[i] = 442442442442442442442442442442442;
        uint64Field3[i] = 442442442442442442442442442442442;
        sint32Field3[i] = 888888888;
        sint64Field3[i] = -4242424242424242;
        fixed32Field3[i] = 442;
        fixed64Field3[i] = 442442442442442442442442442442442442442442442442442442442442442442;
        sfixed32Field3[i] = -888888888;
        sfixed64Field3[i] = 442442442442442442442442442442442442442442442442442442442442442442;
        boolField3[i] = true;
        stringField3[i] = '42';
        enumField3[i] = Test1.Enum2.ENUM_VALUE_2;
      }
      msg.doubleField3(doubleField3);
      msg.floatField3(floatField3);
      msg.int32Field3(int32Field3);
      msg.uint32Field3(uint32Field3);
      msg.sint32Field3(sint32Field3);
      msg.fixed32Field3(fixed32Field3);
      msg.sfixed32Field3(sfixed32Field3);

      msg.int64Field3(int64Field3);
      msg.uint64Field3(uint64Field3);
      msg.sint64Field3(sint64Field3);
      msg.fixed64Field3(fixed64Field3);
      msg.sfixed64Field3(sfixed64Field3);
      msg.boolField3(boolField3);
      msg.stringField3(stringField3);
      msg.enumField3(enumField3);

      return msg;
    }

    function createSampleMessage() {
      var msg = new Test1.Message2({
        // This one contains large arrays.
        messageField1: createSubMessage(),
      });
      // Many small messages.
      for (var i = 0; i < numMessage; ++i) {
        msg.addMessageField2(createSubMessage(4));
      }
      return msg;
    }

    function initTestCase() {
      sampleMessage = createSampleMessage();
    }

    property var sampleMessage

    function benchmark_ctor() {
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = createSampleMessage();
      }
    }

    function benchmark_v4() {
      var called = {called: 0};
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = sampleMessage;
        var buf = msg1.serialize();
        var msg2 = Test1.Message2.parse(buf);
        ++called.called;
      }
      tryCompare(called, 'called', numLoop);
    }

    function benchmark_v4cb() {
      var printed = false;
      var called = {called: 0};
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = sampleMessage;
        msg1.serialize(function(err, buf) {
          if (!printed) {
            printed = true;
            console.log('PB LEN : ' + buf.byteLength);
          }
          err && console.warn(err);
          Test1.Message2.parse(buf, function(err, msg2) {
            err && console.warn(err);
            ++called.called;
          });
        });
      }
      tryCompare(called, 'called', numLoop);
    }

    function benchmark_lecagy() {
      var called = {called: 0};
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = sampleMessage;
        msg1.serializeTo(buffer.output, function(err) {
          err && console.warn(err);
          Test1.Message2.parseFrom(buffer.input, function(err, msg2) {
            err && console.warn(err);
            ++called.called;
          });
        });
      }
      tryCompare(called, 'called', numLoop);
    }
  }

  TestCase {
    name: 'JsonBenchmark'
    property alias numLoop: root.numLoop
    property alias numMessage: root.numMessage
    property alias numPrimitive: root.numPrimitive

    function createSubJson(size) {
      if (typeof size == 'undefined') {
        size = numPrimitive;
      }
      var msg = {
        setDoubleField1: 42.42,
        setFloatField1: 42.42,
        setInt32Field1: 42,
        setInt64Field1: 42,
        setUint32Field1: 42,
        setUint64Field1: 42,
        setSint32Field1: 42,
        setSint64Field1: 42,
        setFixed32Field1: 42,
        setFixed64Field1: 42,
        setSfixed32Field1: -888888888,
        setSfixed64Field1: 42,
        setBoolField1: true,
        setStringField1: '42',
        setEnumField1: Test1.Enum2.ENUM_VALUE_2,
        setDoubleField2: -42.42,
        setFloatField2: -42.42,
        setInt32Field2: 42,
        setInt64Field2: -999930000000042,
        setUint64Field2: 999930000000042,
        setSfixed32Field2: -888888888,
        setSfixed64Field2: -999930000000042,
        setEnumField2: Test1.Enum2.ENUM_VALUE_3,
        setDoubleField3: [442.42, -2.42, 442.42, -2.42],
        setFloatField3: [442.42, -2.42, 442.42, -2.42],
        setInt32Field3: [442, -2, 442, -2],
        setInt64Field3: [442, -999999999992, 442, -999999999992],
        setUint32Field3: [442, 2, 442, 2],
        setUint64Field3: [442, 999999999992, 442, 999999999992],
        setSint32Field3: [442, -2, 442, -2],
        setSint64Field3: [442, -999999999992, 442, -999999999992],
        setFixed32Field3: [442, 2, 442, 2],
        setFixed64Field3: [442, 999999999992, 442, 999999999992],
        setSfixed32Field3: [442, -2, 442, -2],
        setSfixed64Field3: [442, -999999999992, 442, -999999999992],
        setBoolField3: [true, false, true],
        setStringField3: ['42', 'foo', 'bar'],
        setEnumField3: [Test1.Enum2.ENUM_VALUE_2, Test1.Enum2.ENUM_VALUE_4, Test1.Enum2.ENUM_VALUE_3],
        doubleField3: new Array(size),
        floatField3: new Array(size),
        int32Field3: new Array(size),
        int64Field3: new Array(size),
        uint32Field3: new Array(size),
        uint64Field3: new Array(size),
        sint32Field3: new Array(size),
        sint64Field3: new Array(size),
        fixed32Field3: new Array(size),
        fixed64Field3: new Array(size),
        sfixed32Field3: new Array(size),
        sfixed64Field3: new Array(size),
        boolField3: new Array(size),
        stringField3: new Array(size),
        enumField3: new Array(size),
      };
      for (var i = 0; i < size; ++i) {
        msg.doubleField3[i] = 1234134.1341234;
        msg.floatField3[i] = 1234134.1341234;
        msg.int32Field3[i] = 442;
        msg.int64Field3[i] = 442442442442442442442442442442442;
        msg.uint32Field3[i] = 442;
        msg.uint64Field3[i] = 442442442442442442442442442442442;
        msg.sint32Field3[i] = 888888888;
        msg.sint64Field3[i] = -4242424242424242;
        msg.fixed32Field3[i] = 888888888;
        msg.fixed64Field3[i] = 442442442442442442442442442442442442442442442442442442442442442442;
        msg.sfixed32Field3[i] = -888888888;
        msg.sfixed64Field3[i] = 442442442442442442442442442442442442442442442442442442442442442442;
        msg.boolField3[i] = true;
        msg.stringField3[i] = '42';
        msg.enumField3[i] = Test1.Enum2.ENUM_VALUE_2;
      }
      return msg;
    }

    function createSampleJson() {
      var msg = {
        // This one contains large arrays.
        messageField1: createSubJson(),
        messageField2: [],
      };
      // Many small messages.
      for (var i = 0; i < numMessage; ++i) {
        msg.messageField2.push(createSubJson(4));
      }
      return msg;
    }

    property var sampleJson

    function initTestCase() {
      sampleJson = createSampleJson();
    }

    function benchmark_json_ctor() {
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = createSampleJson();
      }
    }

    function benchmark_json() {
      var printed = false;
      for (var i = 0; i < numLoop; ++i) {
        var j = sampleJson;
        var str = JSON.stringify(j);
        if (!printed) {
          printed = true;
          console.log('JSON LEN : ' + str.length);
        }
        var json2 = JSON.parse(str);
      }
    }
  }
}
