import QtQuick 2.2
import QtTest 1.0
import Protobuf 1.0 as Protobuf
import 'oneof_test.pb.js' as Msg

Item {
  Protobuf.MemoryBuffer {
    id: buffer
  }

  TestCase {
    name: 'OneofFieldTest'

    function init() {
      buffer.clear();
      buffer.size = 1000;
    }

    function test_simple() {
      // when initialized with JS object
      var msg1 = new Msg.Foo({
        oneof2: 'foo',
      });

      // oneof virtual field should return case enum
      compare(msg1.simpleCase(), Msg.Foo.SimpleCase.ONEOF2);

      // value should be available
      compare(msg1.oneof2(), 'foo');

      // serialize -> parse
      var called = {};
      msg1.serializeTo(buffer.output, function() {
        Msg.Foo.parseFrom(buffer.input, function(msg2) {
          compare(msg2.simpleCase(), Msg.Foo.SimpleCase.ONEOF2);
          compare(msg2.oneof2(), 'foo');
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_message() {
      // when initialized with JS object
      var msg1 = new Msg.Foo({
        oneof3: {
          baz: 'baz!',
        },
      });

      // oneof virtual field should return case enum
      compare(msg1.simpleCase(), Msg.Foo.SimpleCase.ONEOF3);

      // value should be available
      compare(msg1.oneof3().baz(), 'baz!');

      // serialize -> parse
      var called = {};
      msg1.serializeTo(buffer.output, function() {
        Msg.Foo.parseFrom(buffer.input, function(msg2) {
          compare(msg2.simpleCase(), Msg.Foo.SimpleCase.ONEOF3);
          compare(msg2.oneof3().baz(), 'baz!');
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_nested() {
      var msg1 = new Msg.Foo({
        baz: {
          baz2: {
            barStr2: 'foo',
            barStr3: 'bar',
          },
        },
      });

      compare(msg1.baz().bazXCase(), Msg.Foo.Baz.BazXCase.BAZ2);
      compare(msg1.baz().baz2().bar1Case(), Msg.Bar.Bar1Case.BAR_STR2);
      compare(msg1.baz().baz2().bar2Case(), Msg.Bar.Bar2Case.BAR_STR3);
      compare(msg1.baz().baz2().barStr2(), 'foo');
      compare(msg1.baz().baz2().barStr3(), 'bar');

      // serialize -> parse
      var called = {};
      msg1.serializeTo(buffer.output, function() {
        Msg.Foo.parseFrom(buffer.input, function(msg2) {
          compare(msg2.baz().bazXCase(), Msg.Foo.Baz.BazXCase.BAZ2);
          compare(msg2.baz().baz2().bar1Case(), Msg.Bar.Bar1Case.BAR_STR2);
          compare(msg2.baz().baz2().bar2Case(), Msg.Bar.Bar2Case.BAR_STR3);
          compare(msg2.baz().baz2().barStr2(), 'foo');
          compare(msg2.baz().baz2().barStr3(), 'bar');
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_init() {
      var foo = new Msg.Foo();

      compare(foo.simpleCase(), Msg.Foo.SimpleCase.SIMPLE_NOT_SET);
      compare(foo.oneof1(), undefined);
      compare(foo.oneof2(), undefined);
      compare(foo.oneof3(), undefined);

      // then serialize -> parse
      var called = {};
      foo.serializeTo(buffer.output, function() {
        Msg.Foo.parseFrom(buffer.input, function(msg2) {
          compare(msg2.simpleCase(), Msg.Foo.SimpleCase.SIMPLE_NOT_SET);
          compare(msg2.oneof1(), undefined);
          compare(msg2.oneof2(), undefined);
          compare(msg2.oneof3(), undefined);
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }

    function test_clear() {
      var foo = new Msg.Foo({});

      foo.clearSimple();

      compare(foo.simpleCase(), Msg.Foo.SimpleCase.SIMPLE_NOT_SET);
      compare(foo.oneof1(), undefined);
      compare(foo.oneof2(), undefined);
      compare(foo.oneof3(), undefined);
    }

    function test_overwrite() {
      var foo = new Msg.Foo({});

      foo.oneof1(60);
      compare(foo.simpleCase(), Msg.Foo.SimpleCase.ONEOF1);
      compare(foo.oneof1(), 60);
      compare(foo.oneof2(), undefined);
      compare(foo.oneof3(), undefined);

      foo.oneof2('Hello.');
      compare(foo.simpleCase(), Msg.Foo.SimpleCase.ONEOF2);
      compare(foo.oneof1(), undefined);
      compare(foo.oneof2(), 'Hello.');
      compare(foo.oneof3(), undefined);

      foo.oneof3({
        barStr4: -20,
      });
      compare(foo.simpleCase(), Msg.Foo.SimpleCase.ONEOF3);
      compare(foo.oneof1(), undefined);
      compare(foo.oneof2(), undefined);
      compare(foo.oneof3().barStr4(), -20);

      foo.oneof2('Hello again.');
      compare(foo.simpleCase(), Msg.Foo.SimpleCase.ONEOF2);
      compare(foo.oneof1(), undefined);
      compare(foo.oneof2(), 'Hello again.');
      compare(foo.oneof3(), undefined);

      // then serialize -> parse
      var called = {};
      foo.serializeTo(buffer.output, function() {
        Msg.Foo.parseFrom(buffer.input, function(msg2) {
          compare(msg2.simpleCase(), Msg.Foo.SimpleCase.ONEOF2);
          compare(msg2.oneof1(), undefined);
          compare(msg2.oneof2(), 'Hello again.');
          compare(msg2.oneof3(), undefined);
          called.called = true;
        });
      });
      tryCompare(called, 'called', true, 100);
    }
  }
}
