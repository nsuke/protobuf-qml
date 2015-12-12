import QtQuick 2.5
import Protobuf 1.0 as PB

PB.ServerUnaryMethodHolder {
  id: root
  property var handler
  property var readType
  property var writeType

  readDescriptor: (readType && readType.descriptor) || null
  writeDescriptor: (writeType && writeType.descriptor) || null

  onData: {
    'use strict';
    if (!handler) {
      console.warn('[' + root.methodName + ']: Service handler is not available. Please provide handler implementation.');
      return;
    }
    var msg = data instanceof root.readType ? data : new root.readType(data);
    handler(msg, function(err, response) {
      if (err) {
        if (typeof err === 'string') {
          root.abort(tag, PB.StatusCode.UNKNOWN, err);
        } else {
          root.abort(tag, err.code || PB.StatusCode.UNKNOWN, err.message);
        }

      } else {
        var msg = response instanceof root.writeType ? response : new root.writeType(response);
        root.respond(tag, msg._raw);
      }
    });
  }
}
