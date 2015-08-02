import QtQuick 2.5
import QtTest 1.1
import Protobuf 1.0 as Protobuf
import 'serialization_test.pb.js' as Test1

Item {
  TestCase {
    name: 'SerializationTest'

    function test_sync_ctor() {
      var msg = new Test1.Message1({
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
        sfixed32Field1: 42,
        sfixed64Field1: 42,
        boolField1: true,
        stringField1: '42',
        enumField1: Test1.Enum2.ENUM_VALUE_2,

        doubleField2: -42.42,
        floatField2: -42.42,
        int32Field2: -42,
        int64Field2: -999930000000042,
        uint64Field2: 999930000000042,
        sfixed32Field2: -42,
        sfixed64Field2: -999930000000042,
        enumField2: Test1.Enum2.ENUM_VALUE_3,

        doubleField3: [442.42, -2.42, 442.42, -2.42],
        floatField3: [442.42, -2.42, 442.42, -2.42],
        int32Field3: [442, -2, 442, -2],
        int64Field3: [442, -999999999992, 442, -999999999992],
        uint32Field3: [442, 2, 442, 2],
        uint64Field3: [442, 999999999992, 442, 999999999992],
        sint32Field3: [442, -2, 442, -2],
        sint64Field3: [442, -999999999992, 442, -999999999992],
        fixed32Field3: [442, 2, 442, 2],
        fixed64Field3: [442, 999999999992, 442, 999999999992],
        sfixed32Field3: [442, -2, 442, -2],
        sfixed64Field3: [442, -999999999992, 442, -999999999992],
        boolField3: [true, false, true],
        stringField3: ['42', 'foo', 'bar'],
        enumField3: [Test1.Enum2.ENUM_VALUE_2, Test1.Enum2.ENUM_VALUE_4, Test1.Enum2.ENUM_VALUE_3],
      });
      var buf = msg.serialize();
      verify(buf);
      verify(buf instanceof ArrayBuffer);
      verify(buf.byteLength > 0);

      var msg2 = Test1.Message1.parse(buf);
        compare(msg2.doubleField1, 42.42);
        compare(msg2.floatField1, 42.42);
        compare(msg2.int32Field1, 42);
        compare(msg2.int64Field1, 42);
        compare(msg2.uint32Field1, 42);
        compare(msg2.uint64Field1, 42);
        compare(msg2.sint32Field1, 42);
        compare(msg2.sint64Field1, 42);
        compare(msg2.fixed32Field1, 42);
        compare(msg2.fixed64Field1, 42);
        compare(msg2.sfixed32Field1, 42);
        compare(msg2.sfixed64Field1, 42);
        compare(msg2.boolField1, true);
        compare(msg2.stringField1, '42');
        compare(msg2.enumField1, Test1.Enum2.ENUM_VALUE_2);

        compare(msg2.doubleField2, -42.42);
        compare(msg2.floatField2, -42.42);
        compare(msg2.int32Field2, -42);
        compare(msg2.int64Field2, -999930000000042);
        compare(msg2.uint64Field2, 999930000000042);
        compare(msg2.sfixed32Field2, -42);
        compare(msg2.sfixed64Field2, -999930000000042);
        compare(msg2.enumField2, Test1.Enum2.ENUM_VALUE_3);

        compare(msg2.doubleField3Length, 4);
        compare(msg2.doubleField3(0), 442.42);

        compare(msg2.getInt32Field3AsArray(), [442, -2, 442, -2]);

        compare(msg2.getUint32Field3AsArray(), [442, 2, 442, 2]);
        compare(msg2.getUint64Field3AsArray(), [442, 999999999992, 442, 999999999992]);
        compare(msg2.getSint32Field3AsArray(), [442, -2, 442, -2]);
        compare(msg2.getFixed32Field3AsArray(), [442, 2, 442, 2]);
        compare(msg2.getFixed64Field3AsArray(), [442, 999999999992, 442, 999999999992]);
        compare(msg2.getSfixed32Field3AsArray(), [442, -2, 442, -2]);
        verify(msg2.boolField3(0));
        verify(!msg2.boolField3(1));
        verify(msg2.boolField3(2));
        compare(msg2.getStringField3AsArray(), ['42', 'foo', 'bar']);
        compare(msg2.getEnumField3AsArray(), [Test1.Enum2.ENUM_VALUE_2, Test1.Enum2.ENUM_VALUE_4, Test1.Enum2.ENUM_VALUE_3]);

        fuzzyCompare(msg2.getDoubleField3At(0), 442.42, 0.001);
        fuzzyCompare(msg2.getDoubleField3At(1), -2.42, 0.001);
        fuzzyCompare(msg2.getDoubleField3At(2), 442.42, 0.001);
        fuzzyCompare(msg2.getDoubleField3At(3), -2.42, 0.001);
        fuzzyCompare(msg2.getFloatField3At(0), 442.42, 0.001);
        fuzzyCompare(msg2.getFloatField3At(1), -2.42, 0.001);
        fuzzyCompare(msg2.getFloatField3At(2), 442.42, 0.001);
        fuzzyCompare(msg2.getFloatField3At(3), -2.42, 0.001);

        compare(msg2.getInt64Field3AsArray(), [442, -999999999992, 442, -999999999992]);
        compare(msg2.getSint64Field3AsArray(), [442, -999999999992, 442, -999999999992]);
        compare(msg2.getSfixed64Field3AsArray(), [442, -999999999992, 442, -999999999992]);
    }

    function test_default_value() {
      var msg = new Test1.Message1({
        doubleField2: 42,
        doubleField1: 0,
        floatField1: 0,
        int32Field1: 0,
        int64Field1: 0,
        uint32Field1: 0,
        uint64Field1: 0,
        sint32Field1: 0,
        sint64Field1: 0,
        fixed32Field1: 0,
        fixed64Field1: 0,
        sfixed32Field1: 0,
        sfixed64Field1: 0,
        boolField1: false,
        stringField1: '',
        enumField1: Test1.Enum2.ENUM_VALUE_1,
        doubleField2: 0,
        floatField2: 0,
        int32Field2: 0,
        int64Field2: 0,
        uint32Field2: 0,
        uint64Field2: 0,
        sint32Field2: 0,
        sint64Field2: 0,
        fixed32Field2: 0,
        fixed64Field2: 0,
        sfixed32Field2: 0,
        sfixed64Field2: 0,
        boolField2: false,
        stringField2: '',
        enumField2: Test1.Enum2.ENUM_VALUE_1,
        doubleField3: [],
        floatField3: [],
        int32Field3: [],
        int64Field3: [],
        uint32Field3: [],
        uint64Field3: [],
        sint32Field3: [],
        sint64Field3: [],
        fixed32Field3: [],
        fixed64Field3: [],
        sfixed32Field3: [],
        sfixed64Field3: [],
        boolField3: [],
        stringField3: [],
        enumField3: [],
      });
      var buf = msg.serialize();
      var msg2 = Test1.Message1.parse(buf);
      compare(msg2.doubleField1, 0);
      compare(msg2.floatField1, 0);
      compare(msg2.int32Field1, 0);
      compare(msg2.int64Field1, 0);
      compare(msg2.uint32Field1, 0);
      compare(msg2.uint64Field1, 0);
      compare(msg2.sint32Field1, 0);
      compare(msg2.sint64Field1, 0);
      compare(msg2.fixed32Field1, 0);
      compare(msg2.fixed64Field1, 0);
      compare(msg2.sfixed32Field1, 0);
      compare(msg2.sfixed64Field1, 0);
      compare(msg2.boolField1, false);
      compare(msg2.stringField1, '');
      compare(msg2.enumField1, Test1.Enum2.ENUM_VALUE_1);

      compare(msg2.doubleField2, 0);
      compare(msg2.floatField2, 0);
      compare(msg2.int32Field2, 0);
      compare(msg2.int64Field2, 0);
      compare(msg2.uint32Field2, 0);
      compare(msg2.uint64Field2, 0);
      compare(msg2.sint32Field2, 0);
      compare(msg2.sint64Field2, 0);
      compare(msg2.fixed32Field2, 0);
      compare(msg2.fixed64Field2, 0);
      compare(msg2.sfixed32Field2, 0);
      compare(msg2.sfixed64Field2, 0);
      compare(msg2.boolField2, false);
      compare(msg2.stringField2, '');
      compare(msg2.enumField2, Test1.Enum2.ENUM_VALUE_1);

      compare(msg2.getDoubleField3AsArray(), []);
      compare(msg2.getFloatField3AsArray(), []);
      compare(msg2.getInt32Field3AsArray(), []);
      compare(msg2.getInt64Field3AsArray(), []);
      compare(msg2.getUint32Field3AsArray(), []);
      compare(msg2.getUint64Field3AsArray(), []);
      compare(msg2.getSint32Field3AsArray(), []);
      compare(msg2.getSint64Field3AsArray(), []);
      compare(msg2.getFixed32Field3AsArray(), []);
      compare(msg2.getFixed64Field3AsArray(), []);
      compare(msg2.getSfixed32Field3AsArray(), []);
      compare(msg2.getSfixed64Field3AsArray(), []);
      compare(msg2.getBoolField3AsArray(), []);
      compare(msg2.getStringField3AsArray(), []);
      compare(msg2.getEnumField3AsArray(), []);
    }

    function test_missing_value() {
      var msg = new Test1.Message1({
        doubleField2: 42,
      });
      var buf = msg.serialize();
      var msg2 = Test1.Message1.parse(buf);
      compare(msg2.doubleField1, 0);
      compare(msg2.floatField1, 0);
      compare(msg2.int32Field1, 0);
      compare(msg2.int64Field1, 0);
      compare(msg2.uint32Field1, 0);
      compare(msg2.uint64Field1, 0);
      compare(msg2.sint32Field1, 0);
      compare(msg2.sint64Field1, 0);
      compare(msg2.fixed32Field1, 0);
      compare(msg2.fixed64Field1, 0);
      compare(msg2.sfixed32Field1, 0);
      compare(msg2.sfixed64Field1, 0);
      compare(msg2.boolField1, false);
      compare(msg2.stringField1, '');
      compare(msg2.enumField1, Test1.Enum2.ENUM_VALUE_1);

      compare(msg2.floatField2, 0);
      compare(msg2.int32Field2, 0);
      compare(msg2.int64Field2, 0);
      compare(msg2.uint32Field2, 0);
      compare(msg2.uint64Field2, 0);
      compare(msg2.sint32Field2, 0);
      compare(msg2.sint64Field2, 0);
      compare(msg2.fixed32Field2, 0);
      compare(msg2.fixed64Field2, 0);
      compare(msg2.sfixed32Field2, 0);
      compare(msg2.sfixed64Field2, 0);
      compare(msg2.boolField2, false);
      compare(msg2.stringField2, '');
      compare(msg2.enumField2, Test1.Enum2.ENUM_VALUE_1);

      compare(msg2.getDoubleField3AsArray(), []);
      compare(msg2.getFloatField3AsArray(), []);
      compare(msg2.getInt32Field3AsArray(), []);
      compare(msg2.getInt64Field3AsArray(), []);
      compare(msg2.getUint32Field3AsArray(), []);
      compare(msg2.getUint64Field3AsArray(), []);
      compare(msg2.getSint32Field3AsArray(), []);
      compare(msg2.getSint64Field3AsArray(), []);
      compare(msg2.getFixed32Field3AsArray(), []);
      compare(msg2.getFixed64Field3AsArray(), []);
      compare(msg2.getSfixed32Field3AsArray(), []);
      compare(msg2.getSfixed64Field3AsArray(), []);
      compare(msg2.getBoolField3AsArray(), []);
      compare(msg2.getStringField3AsArray(), []);
      compare(msg2.getEnumField3AsArray(), []);
    }

  }
}
