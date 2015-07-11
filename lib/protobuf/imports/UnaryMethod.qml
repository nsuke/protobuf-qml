import QtQuick 2.3
import Protobuf 1.0 as PB

ClientMethod {
  property alias channel: impl.channel
  property alias methodName: impl.methodName

  _holder: PB.UnaryMethodHolder {
    id: impl
  }

  function call(data, callback, timeout) {
    'use strict';
    if (typeof timeout == 'undefined') {
      timeout = -1;
    }
    var t = ++_storage.tag;
    _storage.addCallback(t, function(err, data) {
      callback && callback(err, new readType(data));
    });
    var ok = impl.write(t, new writeType(data)._raw, timeout);
    if (!ok) {
      console.log('Discarding stored callback.');
      _storage.removeCallback(t);
    }
    return ok;
  }
}
