import QtQuick 2.2
import Protobuf 1.0 as PB

QtObject {
  id: root
  property var channel

  // readonly for those properties does not seem to work for Qt 5.2
  property alias input: root
  property alias output: root

  property var __parse__: PB.UnaryMethod {
    id: parseMethod
    methodName: '/Buffer/Parse'
    channel: root.channel
  }

  property var __serialize__: PB.UnaryMethod {
    id: serializeMethod
    methodName: '/Buffer/Serialize'
    channel: root.channel
  }

  function serialize(type, data, callback) {
    if (!channel) {
      console.warn('Buffer implementation is not available');
      return false;
    }
    serializeMethod.writeType = type;
    serializeMethod.readType = type;
    return serializeMethod.call(data, function (err) {
      callback(err);
    });
  }

  function parse(type, callback) {
    if (!channel) {
      console.warn('Buffer implementation is not available');
      return false;
    }
    parseMethod.writeType = type;
    parseMethod.readType = type;
    return parseMethod.call(undefined, callback);
  }
}
