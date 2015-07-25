import QtQuick 2.5
import QtTest 1.1
import Protobuf 1.0 as Protobuf
import 'recursive_test.pb.js' as Test1

Item {
  Protobuf.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'RecursiveConstructorTest'

    function test_simple_empty() {
      var msg = new Test1.SimpleRecurse();
      compare(msg.next, undefined);
    }

    function test_simple() {
      var msg = new Test1.SimpleRecurse({
        next: {
          next: new Test1.SimpleRecurse({
            value: 42,
          }),
        },
      });
      verify(msg.next);
      verify(msg.next instanceof Test1.SimpleRecurse);
      verify(msg.next.next instanceof Test1.SimpleRecurse);
      compare(msg.next.next.value, 42);

      var buf = msg.serialize();
      var msg2 = Test1.SimpleRecurse.parse(buf);
      verify(msg2.next);
      verify(msg2.next instanceof Test1.SimpleRecurse);
      verify(msg2.next.next instanceof Test1.SimpleRecurse);
      compare(msg2.next.next.value, 42);

    }

    function test_repeated_empty() {
      var msg = new Test1.RepeatedRecurse();
      compare(msg.next, undefined);
    }

    function test_repeated() {
      var msg = new Test1.RepeatedRecurse({
        children: [
          {
            children: [
              new Test1.RepeatedRecurse({
                value: 42,
              }),
            ],
          },
          {
            value: -42,
          },
        ],
      });
      verify(msg.children);
      compare(msg.childrenLength, 2);
      verify(msg.children(0) instanceof Test1.RepeatedRecurse);
      compare(msg.children(0).childrenLength, 1);
      verify(msg.children(0).children(0) instanceof Test1.RepeatedRecurse);
      compare(msg.children(0).children(0).value, 42);
      verify(msg.children(1) instanceof Test1.RepeatedRecurse);
      compare(msg.children(1).value, -42);

      var buf = msg.serialize();
      var msg2 = Test1.RepeatedRecurse.parse(buf);
      verify(msg2.children);
      compare(msg2.childrenLength, 2);
      verify(msg2.children(0) instanceof Test1.RepeatedRecurse);
      compare(msg2.children(0).childrenLength, 1);
      verify(msg2.children(0).children(0) instanceof Test1.RepeatedRecurse);
      compare(msg2.children(0).children(0).value, 42);
      verify(msg2.children(1) instanceof Test1.RepeatedRecurse);
      compare(msg2.children(1).value, -42);
    }

    function test_oneof_empty() {
      var msg = new Test1.OneofRecurse();
      compare(msg.next, undefined);
      compare(msg.value, undefined);
    }

    function test_oneof() {
      var msg = new Test1.OneofRecurse({
        next: new Test1.OneofRecurse(),
      });
      verify(msg.next);
      verify(msg.next instanceof Test1.OneofRecurse);
      compare(msg.value, undefined);

      msg.value = 42;
      compare(msg.next, undefined);
      compare(msg.value, 42);

      msg.next = new Test1.OneofRecurse();
      verify(msg.next);
      verify(msg.next instanceof Test1.OneofRecurse);
      compare(msg.value, undefined);

      var buf = msg.serialize();
      var msg2 = Test1.OneofRecurse.parse(buf);
      verify(msg2.next);
      verify(msg2.next instanceof Test1.OneofRecurse);
      compare(msg2.value, undefined);
    }

    function test_cycle_empty() {
      var msgA = new Test1.CyclicA();
      compare(msgA.peer, undefined);
      var msgB = new Test1.CyclicB();
      compare(msgB.peer, undefined);
    }

    function test_cycle() {
      var msg = new Test1.CyclicA({
        peer: {
          peer: new Test1.CyclicA({
            value: 42,
          }),
        },
      });
      verify(msg.peer instanceof Test1.CyclicB);
      verify(msg.peer.peer instanceof Test1.CyclicA);
      compare(msg.peer.peer.value, 42);

      var buf = msg.serialize();
      var msg2 = Test1.CyclicA.parse(buf);
      verify(msg2.peer instanceof Test1.CyclicB);
      verify(msg2.peer.peer instanceof Test1.CyclicA);
      compare(msg2.peer.peer.value, 42);
    }
  }
}
