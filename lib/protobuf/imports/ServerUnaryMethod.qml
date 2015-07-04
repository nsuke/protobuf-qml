import QtQuick 2.2
import Protobuf 1.0 as PB

PB.ServerUnaryMethodHolder {
  id: root
  property var handler
  property var readType
  property var writeType

  readDescriptor: readType && readType.descriptor
  writeDescriptor: writeType && writeType.descriptor

  onData: {
    'use strict';
    if (typeof handler !== 'function') {
      console.warn('[' + root.methodName + ']: Service handler is not available. Please provide handler implementation.');
      return;
    }
    handler(new root.readType(data), function(err, response) {
      root.respond(tag, new root.writeType(response)._raw);
    });
  }
}
