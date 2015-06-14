import QtQuick 2.2
import Protobuf 1.0 as P

Item {
  property alias channel: impl.channel
  property alias methodName: impl.methodName
  property alias readDescriptor: impl.readDescriptor
  property alias writeDescriptor: impl.writeDescriptor

  P.UnaryMethodHolder {
    id: impl
    onData: p.handleData(tag, data)
    onError: p.handleError(tag, message)
    onClosed: p.handleClosed(tag)
  }

  Item {
    id: p

    property var callbackStorage: []

    function handleClosed(tag) {
      // TODO: find the call and delete
    }

    function addCallback(tag, callback) {
      callbackStorage[tag] = {
        timestamp: Date.now(),
        callback: callback,
      };
    }

    function handleData(tag, data) {
      var call = callbackStorage[tag];
      if (!call) {
        console.warn('Received data for unknown tag: ' + tag);
        return;
      }
      try {
        call.callback(data);
      } finally {
        delete callbackStorage[tag];
      }
    }

    function handleError(tag, err) {
      console.log('Error for tag ' + tag + ': ' + err);
      var call = callbackStorage[tag];
      if (call) {
        try {
          call.callback(undefined, err);
        } finally {
          delete callbackStorage[tag];
        }
      }
    }
    property int tag: 0

    function removeCallback(tag) {
      // TODO:
    }
  }

  function call(data, callback, timeout) {
    if (typeof timeout == 'undefined') {
      timeout = -1;
    }
    var t = ++p.tag;
    p.addCallback(t, callback);
    var ok = impl.write(t, data, timeout);
    if (!ok) {
      console.log('Discarding stored callback.');
      p.removeCallback(t);
    }
    return ok;
  }
}
