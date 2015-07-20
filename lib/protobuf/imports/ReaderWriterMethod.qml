import QtQuick 2.4
import Protobuf 1.0 as P

ClientMethod {
  property alias channel: impl.channel
  property alias methodName: impl.methodName

  _holder: P.ReaderWriterMethodHolder {
    id: impl
    onDataEnd: p.handleDataEnd(tag)
  }

  property var __private__: QtObject {
    id: p

    function handleDataEnd(tag) {
      'use strict';
      var call = _storage.getCallback(tag);
      if (!call) {
        console.warn('Received data for unknown tag: ' + tag);
        return;
      }
      call.callback(null, null, true);
    }

    function write(tag, data) {
      'use strict';
      var call = _storage.getCallback(tag);
      if (!call) {
        console.warn('Call object not found for tag: ' + tag);
        return false;
      }
      var ok = impl.write(tag, data);
      if (!ok) {
        call.callback('Failed to write.');
        _storage.removeCallback(tag);
      }
      return ok;
    }

    function end(tag, timeout) {
      'use strict';
      if (typeof timeout == 'undefined') {
        timeout = -1;
      }
      var call = _storage.getCallback(tag);
      if (!call) {
        console.warn('Call object not found for tag: ' + tag);
        return false;
      }
      var ok = impl.writesDone(tag, timeout);
      if (!ok) {
        call.callback('Failed to end writing.');
        _storage.removeCallback(tag);
      }
      return ok;
    }
  }

  function call(callback) {
    'use strict';
    var t = _storage.nextTag();
    _storage.addCallback(t, function(err, data, finished) {
      callback && callback(err, toReadMessage(data), finished);
    });
    impl.call(t);
    return {
      get timeout() {
        return impl.timeout(t);
      },
      set timeout(val) {
        return impl.set_timeout(t, val);
      },
      write: function(data) {
        return p.write(t, toWriteMessage(data)._raw);
      },
      end: function() {
        return p.end(t);
      },
    };
  }
}
