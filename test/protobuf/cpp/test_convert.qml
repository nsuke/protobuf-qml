import QtQuick 2.5
import SomeModule 1.0
import 'test_convert.pb.js' as PB

Item {
  SomeQmlType {
    id: t
  }

  Item {
    objectName: 'ConvertTest'

    property string testOnResult
    property string testForResult

    Component.onCompleted: {
      test_for();
      test_on();
    }

    function test_on() {
      var msg1 = new PB.Message1({
        intField1: -42,
      });
      var msg2 = new PB.Message2(t.useOn(msg1));
      testOnResult = msg2.textField2;
    }

    function test_for() {
      var msg1 = new PB.Message1({
        intField1: 999999999,
      });
      var msg2 = new PB.Message2(t.useFor(msg1));
      testForResult = msg2.textField2;
    }
  }
}
