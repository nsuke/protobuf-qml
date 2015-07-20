import QtQuick 2.5
import QtTest 1.1
import Protobuf 1.0 as Protobuf
import 'ProtobufQmlTest.pb.js' as Test1

Item {
  Protobuf.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'BytesFieldTest'

    property var buf

    function initTestCase() {
      var arr = new Int32Array(30000);
      for (var i = 0; i < arr.length; ++i) {
        arr[i] = i + i;
      }
      buf = arr.buffer;
    }

    function test_binary_legacy() {
      var called = {};
      var msg1 = new Test1.Msg1({bytesField1: buf});
      msg1.serializeTo(buffer.output, function(err) {
        verify(!err);
        Test1.Msg1.parseFrom(buffer.input, function(err, msg2) {
          verify(!err);
          verify(msg2);
          verify(msg2.bytesField1);
          var view = new Int32Array(msg2.bytesField1);
          for (var i = 0; i < view.length; ++i) {
            compare(view[i], i + i);
          }
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_binary_v4cb() {
      var called = {};
      var msg1 = new Test1.Msg1({bytesField1: buf});
      msg1.serialize(function(err, buf0) {
        // console.log('cb : ' + buf0.byteLength);
        verify(!err);
        Test1.Msg1.parse(buf0, function(err, msg2) {
          verify(!err);
          verify(msg2);
          verify(msg2.bytesField1);
          var view = new Int32Array(msg2.bytesField1);
          for (var i = 0; i < view.length; ++i) {
            compare(view[i], i + i);
          }
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 500);
    }

    function test_binary_v4() {
      var called = {};
      var msg1 = new Test1.Msg1({bytesField1: buf});
      var buf0 = msg1.serialize();
        var msg2 = Test1.Msg1.parse(buf0);
          verify(msg2);
          verify(msg2.bytesField1);
          var view = new Int32Array(msg2.bytesField1);
          for (var i = 0; i < view.length; ++i) {
            compare(view[i], i + i);
          }
          called.called = true;
      tryCompare(called, 'called', true, 100);
    }
  }
}
