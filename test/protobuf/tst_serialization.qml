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
        int64Field2: -930000000042,
        uint64Field2: 930000000042,
        sfixed32Field2: -42,
        sfixed64Field2: -930000000042,
        enumField2: Test1.Enum2.ENUM_VALUE_3,

        doubleField3: [442.42, -2.42, 442.42, -2.42],
        floatField3: [442.42, -2.42, 442.42, -2.42],
        int32Field3: [442, -2, 442, -2],
        int64Field3: [442, -2, 442, -2],
        uint32Field3: [442, -2, 442, -2],
        uint64Field3: [442, -2, 442, -2],
        fixed32Field3: [442, -2, 442, -2],
        fixed64Field3: [442, -2, 442, -2],
        sfixed32Field3: [442, -2, 442, -2],
        sfixed64Field3: [442, -2, 442, -2],
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
        compare(msg2.int64Field2, -930000000042);
        compare(msg2.uint64Field2, 930000000042);
        compare(msg2.sfixed32Field2, -42);
        compare(msg2.sfixed64Field2, -930000000042);
        compare(msg2.enumField2, Test1.Enum2.ENUM_VALUE_3);
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
        fixed32Field1: 0,
        fixed64Field1: 0,
        sfixed32Field1: 0,
        sfixed64Field1: 0,
        boolField1: false,
        stringField1: '',
        enumField1: Test1.Enum2.ENUM_VALUE_1,
      });
      var buf = msg.serialize();
      var msg2 = Test1.Message1.parse(buf);
      compare(msg2.doubleField1, 0);
      compare(msg2.floatField1, 0);
      compare(msg2.int32Field1, 0);
      compare(msg2.int64Field1, 0);
      compare(msg2.uint32Field1, 0);
      compare(msg2.uint64Field1, 0);
      compare(msg2.fixed32Field1, 0);
      compare(msg2.fixed64Field1, 0);
      compare(msg2.sfixed32Field1, 0);
      compare(msg2.sfixed64Field1, 0);
      compare(msg2.boolField1, false);
      compare(msg2.stringField1, '');
      compare(msg2.enumField1, Test1.Enum2.ENUM_VALUE_1);
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
      compare(msg2.fixed32Field1, 0);
      compare(msg2.fixed64Field1, 0);
      compare(msg2.sfixed32Field1, 0);
      compare(msg2.sfixed64Field1, 0);
      compare(msg2.boolField1, false);
      compare(msg2.stringField1, '');
      compare(msg2.enumField1, Test1.Enum2.ENUM_VALUE_1);
    }

  }
}
