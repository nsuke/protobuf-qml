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
    name: 'ParseTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_parse_undefined() {
      var called = {};
      Test1.Msg1.parseFrom(undefined, function(msg){
        console.log('msg ' + msg._raw);
        fail();
      }, function(err) {
        console.log(err);
        verify('error ' + err);
        called.value = true;
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_parse_empty() {
      var called = {};
      Test1.Msg1.parseFrom(buffer.input, function(msg){
        console.log('msg ' + msg._raw);
        fail();
      }, function(err) {
        console.log(err);
        verify('error ' + err);
        called.value = true;
      });
      tryCompare(called, 'value', true, 100);
    }

    function test_parse_size0() {
      var called = {};
      var msg1 = new Test1.Msg1({field1: -42});
      msg1.serializeTo(buffer.output, function() {
        buffer.size = 0;
        Test1.Msg1.parseFrom(buffer.input, function(msg) {
          console.log('msg ' + msg._raw);
          fail();
        }, function(err) {
          console.log(err);
          verify('error ' + err);
          called.value = true;
        });
      });
      tryCompare(called, 'value', true, 100);
    }
  }
}
