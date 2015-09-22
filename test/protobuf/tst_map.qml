import QtQuick 2.5
import QtTest 1.1
import Protobuf 1.0
import 'maps.pb.js' as M

Item {
  TestCase {
    function test_empty() {
      var msg = new M.MessageWithMap();
      var buf = msg.serialize();
      var msg2 = M.MessageWithMap.parse(buf);
      compare(msg2.stringMapSize, 0);
    }

    function test_explicit_api() {
      var msg = new M.MessageWithMap({
        stringMap: [
          {key: 'foo', value: '42'},
          {key: 'bar', value: '420'},
        ],
      });
      msg.addStringMap({ key: 'baz', value: '4200'});
      var buf = msg.serialize();
      var msg2 = M.MessageWithMap.parse(buf);
      compare(msg2.stringMapSize, 3);
      for (var i = 0; i < msg2.stringMapSize; ++i) {
        var entry = msg2.stringMap(i);
        switch (entry.key) {
          case 'foo': compare(entry.value, '42'); break;
          case 'bar': compare(entry.value, '420'); break;
          case 'baz': compare(entry.value, '4200'); break;
          default: fail();
        }
      }
    }

    function test_natural_api() {
      skip('TODO: implement natural API for map fields');
      var msg = new M.MessageWithMap({
        stringMap: {
          'foo': '42',
          'bar': '420',
        },
      });
      msg.addStringMapEntry('baz', '4200');
      var buf = msg.serialize();
      var msg2 = M.MessageWithMap.parse(buf);
      compare(msg2.stringMapSize, 3);
      compare(msg2.stringMapEntry('foo', '42'));
      compare(msg2.stringMapEntry('bar', '420'));
      compare(msg2.stringMapEntry('baz', '4200'));
    }
  }
}
