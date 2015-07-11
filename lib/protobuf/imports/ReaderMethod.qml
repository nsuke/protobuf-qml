import QtQuick 2.3
import Protobuf 1.0 as P

ClientMethod {
  property alias channel: impl.channel
  property alias methodName: impl.methodName

  _holder: P.ReaderMethodHolder {
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
      try {
        call.callback(null, null, true);
      } finally {
        _storage.removeCallback(tag);
      }
    }
  }

  function call(data, callback, timeout) {
    'use strict';
    if (typeof timeout == 'undefined') {
      timeout = -1;
    }
    var t = ++_storage.tag;
    _storage.addCallback(t, function(err, data, end) {
      callback && callback(err, new readType(data), end);
    });
    var ok = impl.write(t, new writeType(data)._raw, timeout);
    if (!ok) {
      _storage.removeCallback(t);
    }
    return ok;
  }
}
