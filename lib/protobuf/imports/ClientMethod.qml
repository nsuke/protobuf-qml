import QtQuick 2.4
import Protobuf 1.0 as PB

QtObject {
  property var readType
  property var writeType

  property var _holder

  property var _bind0: Binding {
    target: _holder
    property: 'readDescriptor'
    value: (readType && readType.descriptor) || null
  }
  property var _bind1: Binding {
    target: _holder
    property: 'writeDescriptor'
    value: (writeType && writeType.descriptor) || null
  }
  property var _connect0: Connections {
    target: _holder
    onData: _storage.handleData(tag, data)
    onError: _storage.handleError(tag, code, message)
    onClosed: _storage.handleClosed(tag)
    }

  property var _storage: QtObject {
    property int _tag: 0
    property var callbackStorage: []

    function nextTag() {
      return ++_tag;
    }

    function addCallback(tag, callback) {
      'use strict';
      callbackStorage[tag] = {
        timestamp: Date.now(),
        callback: callback,
      };
    }

    function removeCallback(tag) {
      'use strict';
      delete callbackStorage[tag];
    }

    function getCallback(tag) {
      'use strict';
      var cb = callbackStorage[tag];
      if (!cb) {
        console.warn('Unknown tag : ' + tag);
      }
      return cb;
    }

    function handleClosed(tag) {
      'use strict';
      removeCallback(tag);
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
      call.callback(null, null, true);
    }

    function handleError(tag, code, message) {
      'use strict';
      if (!code) {
        code = PB.StatusCode.UNKNOWN;
      }
      if (!code) {
        throw new Error('Unable to define error code');
      }
      if (!message) {
        message = 'Unknown error';
      }
      var call = callbackStorage[tag];
      if (call) {
        try {
          var err = PB.RpcErrors.forStatusCode(code);
          call.callback(new err(message));
        } finally {
          removeCallback(tag);
        }
      } else {
        console.warn('Error received for unknown tag: ' + tag + ', Error: (' + code + ') ' + message );
      }
    }
  }
}
