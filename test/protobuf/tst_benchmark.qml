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
  property int messageCount: 250
  property int arrayLength: 10000

  property var testData: [
    {
      tag: 'manyMessages',
      msgs: true,
      arrays: false,
    },
    {
      tag: 'largeArrays',
      msgs: false,
      arrays: true,
    },
    {
      tag: 'both',
      msgs: true,
      arrays: true,
    },
  ]

  TestCase {
    readonly property bool profiling: false

    id: test
    name: 'SerializationBenchmark'
    property alias numLoop: root.numLoop
    property alias messageCount: root.messageCount
    property alias arrayLength: root.arrayLength

    property var manyMessages
    property var largeArrays
    property var both

    function createSubMessage(size, array, typedArray, reserve, resize) {
      if (typeof size == 'undefined') {
        size = arrayLength;
      }
      if (typeof size != 'number') {
        throw new TypeError(typeof size);
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

      if (!size) {
        return msg;
      }

      var array = typeof array == 'undefined' ? true : array;
      var typedArray = typeof typedArray == 'undefined' ? true : typedArray;
      var reserve = typeof reserve == 'undefined' ? false : reserve;
      var resize = typeof resize == 'undefined' ? false : resize;

      if (array) {
        var int64Field3 = new Array(size);
        var uint64Field3 = new Array(size);
        var sint64Field3 = new Array(size);
        var fixed64Field3 = new Array(size);
        var sfixed64Field3 = new Array(size);
        var stringField3 = new Array(size);
      }

      if (typedArray) {
        var doubleField3 = new Float64Array(size);
        var floatField3 = new Float32Array(size);
        var int32Field3 = new Int32Array(size);
        var uint32Field3 = new Uint32Array(size);
        var sint32Field3 = new Int32Array(size);
        var fixed32Field3 = new Uint32Array(size);
        var sfixed32Field3 = new Int32Array(size);
        var boolField3 = new Int8Array(size);
        var enumField3 = new Int32Array(size);
      } else if (reserve) {
        msg.reserveDoubleField3(size);
        msg.reserveFloatField3(size);
        msg.reserveInt32Field3(size);
        msg.reserveUint32Field3(size);
        msg.reserveSint32Field3(size);
        msg.reserveFixed32Field3(size);
        msg.reserveSfixed32Field3(size);
        msg.reserveBoolField3(size);
        msg.reserveEnumField3(size);
      } else if (resize) {
        msg.resizeDoubleField3(size);
        msg.resizeFloatField3(size);
        msg.resizeInt32Field3(size);
        msg.resizeUint32Field3(size);
        msg.resizeSint32Field3(size);
        msg.resizeFixed32Field3(size);
        msg.resizeSfixed32Field3(size);
        msg.resizeBoolField3(size);
        msg.resizeEnumField3(size);
      }

      for (var i = 0; i < size; ++i) {
        if (array) {
          int64Field3[i] = 442442442442442442442442442442442;
          uint64Field3[i] = 442442442442442442442442442442442;
          sint64Field3[i] = -4242424242424242;
          fixed64Field3[i] = 442442442442442442442442442442442442442442442442442442442442442442;
          sfixed64Field3[i] = 442442442442442442442442442442442442442442442442442442442442442442;
          stringField3[i] = '42';
        } else {
          msg.addInt64Field3(442442442442442442442442442442442);
          msg.addUint64Field3(442442442442442442442442442442442);
          msg.addSint64Field3(-4242424242424242);
          msg.addFixed64Field3(442442442442442442442442442442442442442442442442442442442442442442);
          msg.addSfixed64Field3(442442442442442442442442442442442442442442442442442442442442442442);
          msg.addStringField3('42');
        }

        if (typedArray) {
          doubleField3[i] = 1234134.1341234;
          floatField3[i] = 1234134.1341234;
          int32Field3[i] = 442;
          uint32Field3[i] = 442;
          sint32Field3[i] = 888888888;
          fixed32Field3[i] = 442;
          sfixed32Field3[i] = -888888888;
          boolField3[i] = true;
          enumField3[i] = Test1.Enum2.ENUM_VALUE_2;
        } else if (resize) {
          msg.setDoubleField3At(i, 1234134.1341234);
          msg.setFloatField3At(i, 1234134.1341234);
          msg.setInt32Field3At(i, 442);
          msg.setUint32Field3At(i, 442);
          msg.setSint32Field3At(i, 888888888);
          msg.setFixed32Field3At(i, 442);
          msg.setSfixed32Field3At(i, -888888888);
          msg.setBoolField3At(i, true);
          msg.setEnumField3At(i, Test1.Enum2.ENUM_VALUE_2);
        } else {
          msg.addDoubleField3(1234134.1341234);
          msg.addFloatField3(1234134.1341234);
          msg.addInt32Field3(442);
          msg.addUint32Field3(442);
          msg.addSint32Field3(888888888);
          msg.addFixed32Field3(442);
          msg.addSfixed32Field3(-888888888);
          msg.addBoolField3(true);
          msg.addEnumField3(Test1.Enum2.ENUM_VALUE_2);
        }
      }

      if (array) {
        msg.setInt64Field3(int64Field3);
        msg.setUint64Field3(uint64Field3);
        msg.setSint64Field3(sint64Field3);
        msg.setFixed64Field3(fixed64Field3);
        msg.setSfixed64Field3(sfixed64Field3);
        msg.setStringField3(stringField3);
      }

      if (typedArray) {
        msg.setDoubleField3(doubleField3);
        msg.setFloatField3(floatField3);
        msg.setInt32Field3(int32Field3);
        msg.setUint32Field3(uint32Field3);
        msg.setSint32Field3(sint32Field3);
        msg.setFixed32Field3(fixed32Field3);
        msg.setSfixed32Field3(sfixed32Field3);
        msg.setBoolField3(boolField3);
        msg.setEnumField3(enumField3);
      }

      return msg;
    }

    function createSampleMessage(msgs, arrays) {
      var msg = new Test1.Message2();
      if (arrays) {
        // This one contains large arrays.
        msg.setMessageField1(createSubMessage.apply(
          test, Array.prototype.slice.call(arguments, 2)));
      }
      if (msgs) {
        // Many small messages.
        for (var i = 0; i < messageCount; ++i) {
          msg.addMessageField2(createSubMessage(0));
        }
      }
      return msg;
    }

    function initTestCase() {
      for (var i in testData) {
        var data = testData[i];
        test[data.tag] = createSampleMessage(data.msgs, data.arrays);
      }
    }

    function cleanupTestCase() {
      if (profiling) {
        // To prevent profiler from failing.
        wait(50000);
      }
    }

    function benchmark_ctor_data() {
      var configurations = [
        {
          tag: 'optimal',
        },
        {
          tag: 'none',
          array: false,
          typedArray: false,
          reserve: false,
          resize: false,
        },
        {
          tag: 'array',
          array: true,
          typedArray: false,
          reserve: false,
          resize: false,
        },
        {
          tag: 'typedArray',
          array: false,
          typedArray: true,
          reserve: false,
          resize: false,
        },
        {
          tag: 'reserve',
          array: false,
          typedArray: false,
          reserve: true,
          resize: false,
        },
        {
          tag: 'resize',
          array: false,
          typedArray: false,
          reserve: false,
          resize: true,
        },
        {
          tag: 'resize-optimal',
          array: true,
          typedArray: false,
          reserve: false,
          resize: true,
        },
      ];

      var ret = [];
      for (var i in configurations) {
        var c = configurations[i];
        for (var j in root.testData) {
          var td = root.testData[j];
          var merged = {};
          for (var k in c) {
            merged[k] = c[k];
          }
          for (var k in td) {
            merged[k] = td[k];
          }
          merged.tag = c.tag + '-' + td.tag;
          ret.push(merged);
        }
      }
      return ret;
    }
    function benchmark_ctor(data) {
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = createSampleMessage(
          data.msgs,
          data.arrays,
          arrayLength,
          data.array,
          data.typedArray,
          data.reserve,
          data.resize);
      }
    }

    function benchmark_v4_data() { return testData; }
    function benchmark_v4(data) {
      var printed = false;
      var called = {called: 0};
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = test[data.tag];
        var buf = msg1.serialize();
        if (!printed) {
          printed = true;
          console.log('PB LEN : ' + buf.byteLength);
        }
        var msg2 = Test1.Message2.parse(buf);
        ++called.called;
      }
      tryCompare(called, 'called', numLoop);
    }

    function benchmark_v4cb_data() { return testData; }
    function benchmark_v4cb(data) {
      var printed = false;
      var called = {called: 0};
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = test[data.tag];
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

    function benchmark_lecagy_data() { return testData; }
    function benchmark_lecagy(data) {
      var called = {called: 0};
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = test[data.tag];
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
    id: jsonTest
    name: 'JsonBenchmark'
    property alias numLoop: root.numLoop
    property alias messageCount: root.messageCount
    property alias arrayLength: root.arrayLength
    property var both
    property var manyMessages
    property var largeArrays

    function createSubJson(size) {
      if (typeof size == 'undefined') {
        size = arrayLength;
      }
      var msg = {
        doubleField1: 42.42,
        floatField1: 42.42,
        int32Field1: 42,
        int64Field1: 42,
        uint32Field1: 42,
        uint64Field1: 42,
        sint32Field1: 42,
        sint64Field1: 42,
        fixed32Field1: 42,
        fixed64Field1: 42,
        sfixed32Field1: -888888888,
        sfixed64Field1: 42,
        boolField1: true,
        stringField1: '42',
        enumField1: Test1.Enum2.ENUM_VALUE_2,
        doubleField2: -42.42,
        floatField2: -42.42,
        int32Field2: 42,
        int64Field2: -999930000000042,
        uint64Field2: 999930000000042,
        sfixed32Field2: -888888888,
        sfixed64Field2: -999930000000042,
        enumField2: Test1.Enum2.ENUM_VALUE_3,

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

    function createSampleJson(arrays, msgs) {
      var msg = {
        messageField2: [],
      };
      if (arrays) {
        // This one contains large arrays.
        msg['messageField1'] = createSubJson();
      }
      if (msgs) {
        // Many small messages.
        for (var i = 0; i < messageCount; ++i) {
          msg.messageField2.push(createSubJson(0));
        }
      }
      return msg;
    }

    property var sampleJson

    function initTestCase() {
      for (var i in testData) {
        var d = testData[i];
        jsonTest[d.tag] = createSampleJson(d.arrays, d.msgs);
      }
    }

    function benchmark_json_ctor_data() {
      return root.testData;
    }
    function benchmark_json_ctor(data) {
      for (var i = 0; i < numLoop; ++i) {
        var msg1 = createSampleJson(data.arrays, data.msgs);
      }
    }

    function benchmark_json_data() {
      return root.testData;
    }
    function benchmark_json(data) {
      var printed = false;
      for (var i = 0; i < numLoop; ++i) {
        var j = jsonTest[data.tag];
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
