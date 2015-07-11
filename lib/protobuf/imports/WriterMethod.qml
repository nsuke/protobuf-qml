import QtQuick 2.3
import Protobuf 1.0 as P

ClientMethod {
  property alias channel: impl.channel
  property alias methodName: impl.methodName

  _holder: P.WriterMethodHolder {
    id: impl
  }

  property var __private__: QtObject {
    id: p

    function write(tag, data, timeout) {
      'use strict';
      if (typeof timeout == 'undefined') {
        timeout = -1;
      }
      var call = _storage.getCallback(tag);
      if (!call) {
        throw new Error('Call object is invalid.');
      }
      var ok = impl.write(tag, data, timeout);
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
        throw new Error('Call object is invalid.');
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
    _storage.addCallback(t, function(err, data) {
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
