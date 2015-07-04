import QtQuick 2.2
import Protobuf 1.0 as P

Item {
  property alias channel: impl.channel
  property alias methodName: impl.methodName
  property var readType
  property var writeType

  P.WriterMethodHolder {
    id: impl
    readDescriptor: readType && readType.descriptor
    writeDescriptor: writeType && writeType.descriptor
    onData: p.handleData(tag, data)
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
      try {
        call.callback(null, data);
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
        // TODO: notify error
        p.removeCallback(tag);
      }
      return ok;
    }

    function writeEnd(tag, timeout) {
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
        // TODO: notify error
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
      writeEnd: function() {
        return p.writeEnd(t);
      },
    };
  }
}
