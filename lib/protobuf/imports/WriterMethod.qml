import QtQuick 2.2
import Protobuf 1.0 as P

QtObject {
  property alias channel: impl.channel
  property alias methodName: impl.methodName
  property var readType
  property var writeType

  property var holder: P.WriterMethodHolder {
    id: impl
    readDescriptor: (readType && readType.descriptor) || null
    writeDescriptor: (writeType && writeType.descriptor) || null
    onData: p.handleData(tag, data)
    onError: p.handleError(tag, message)
    onClosed: p.handleClosed(tag)
  }

  property var __private__: QtObject {
    id: p

    property var callbackStorage: []

    function handleClosed(tag) {
      'use strict';
      removeCallback(tag);
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
      try {
        call.callback(null, data);
      } finally {
        removeCallback(tag);
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
          removeCallback(tag);
        }
      }
    }
    property int tag: 0

    function removeCallback(tag) {
      'use strict';
      callbackStorage.shift(tag, 1);
    }

    function write(tag, data, timeout) {
      'use strict';
      if (typeof timeout == 'undefined') {
        timeout = -1;
      }
      var call = callbackStorage[tag];
      if (!call) {
        throw new Error('Call object is invalid.');
      }
      var ok = impl.write(tag, data, timeout);
      if (!ok) {
        call.callback('Failed to write.');
        p.removeCallback(tag);
      }
      return ok;
    }

    function end(tag, timeout) {
      'use strict';
      if (typeof timeout == 'undefined') {
        timeout = -1;
      }
      var call = callbackStorage[tag];
      if (!call) {
        throw new Error('Call object is invalid.');
      }
      var ok = impl.writesDone(tag, timeout);
      if (!ok) {
        call.callback('Failed to end writing.');
        p.removeCallback(tag);
      }
      return ok;
    }
  }

  function call(callback) {
    'use strict';
    var t = ++p.tag;
    p.addCallback(t, function(err, data) {
      callback && callback(err, new readType(data));
    });
    return {
      get timeout() {
        return impl.timeout(t);
      },
      set timeout(val) {
        return impl.set_timeout(t, val);
      },
      write: function(data) {
        return p.write(t, new writeType(data)._raw);
      },
      end: function() {
        return p.end(t);
      },
    };
  }
}
