import QtQuick 2.2
import Protobuf 1.0 as PB

Item {
  id: root
  property var channel

  // readonly for those properties does not seem to work for Qt 5.2
  property alias input: root
  property alias output: root

  PB.UnaryMethod {
    id: parseMethod
    methodName: '/Buffer/Parse'
    channel: root.channel
  }

  PB.UnaryMethod {
    id: serializeMethod
    methodName: '/Buffer/Serialize'
    channel: root.channel
  }

  function serialize(descriptor, data, callback) {
    if (!channel) {
      return false;
    }
    serializeMethod.writeDescriptor = descriptor;
    serializeMethod.readDescriptor = descriptor;
    return serializeMethod.call(data, function (ignore, err) {
      callback(err);
    });
  }

  function parse(descriptor, callback) {
    if (!channel) {
      return false;
    }
    parseMethod.writeDescriptor = descriptor;
    parseMethod.readDescriptor = descriptor;
    return parseMethod.call(data, callback);
  }
}
