import QtQuick 2.2
import Protobuf 1.0 as P

Item {
  property alias channel: impl.channel
  property alias methodName: impl.methodName
  property var readType
  property var writeType

  P.ReaderMethodHolder {
    id: impl
    readDescriptor: (readType && readType.descriptor) || null
    writeDescriptor: (writeType && writeType.descriptor) || null
    onData: p.handleData(tag, data)
    onDataEnd: p.handleDataEnd(tag)
    onError: p.handleError(tag, message)
    onClosed: p.handleClosed(tag)
  }

  Item {
    id: p

    property var callbackStorage: []

    function handleClosed(tag) {
      'use strict';
      // TODO: find the call and delete
    }

    function addCallback(tag, callback) {
      'use strict';
      callbackStorage[tag] = {
        timestamp: Date.now(),
        callback: callback,
      };
    }

    function handleData(tag, data) {
      'use strict';
      var call = callbackStorage[tag];
      if (!call) {
        console.warn('Received data for unknown tag: ' + tag);
        return;
      }
      call.callback(null, data);
    }

    function handleDataEnd(tag) {
      'use strict';
      var call = callbackStorage[tag];
      if (!call) {
        console.warn('Received data for unknown tag: ' + tag);
        return;
      }
      try {
        call.callback(null, null, true);
      } finally {
        delete callbackStorage[tag];
      }
    }

    function handleError(tag, err) {
      'use strict';
      console.log('Error for tag ' + tag + ': ' + err);
      var call = callbackStorage[tag];
      if (call) {
        try {
          call.callback(err);
        } finally {
          delete callbackStorage[tag];
        }
      }
    }
    property int tag: 0

    function removeCallback(tag) {
      'use strict';
      // TODO:
    }
  }

  function call(data, callback, timeout) {
    'use strict';
    if (typeof timeout == 'undefined') {
      timeout = -1;
    }
    var t = ++p.tag;
    p.addCallback(t, function(err, data, end) {
      callback && callback(err, new readType(data), end);
    });
    var ok = impl.write(t, new writeType(data)._raw, timeout);
    if (!ok) {
      p.removeCallback(t);
    }
    return ok;
  }
}
