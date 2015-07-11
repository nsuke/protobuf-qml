import QtQuick 2.3
import Protobuf 1.0 as PB

PB.ServerWriterMethodHolder {
  id: root
  property var handler
  property var readType
  property var writeType
  property var handlers: ({})

  readDescriptor: (readType && readType.descriptor) || null
  writeDescriptor: (writeType && writeType.descriptor) || null

  function getHandler(tag) {
    if (!root.handlers[tag]) {
      if (typeof root.handler !== 'function') {
        console.warn('[' + root.methodName + ']: Service handler is not available. Please provide handler implementation.');
        return;
      }
      var call = {
        error: function(err) {
          if (typeof err === 'string') {
            var message = err;
          } else {
            var message = err.message || 'Unknown error';
          }
          var code = err.code || PB.Errors.INTERNAL;
          return root.abort(tag, code, message);
        },
        write: function(response) {
          root.respond(tag, new root.writeType(response)._raw);
        },
        end: function() {
          root.end(tag);
        },
        on: function(key, fn) {
          if (typeof key !== 'string') {
            throw new TypeError('Expected string. Received ' + typeof key);
          }
          if (typeof fn !== 'function') {
            throw new TypeError(
              'Expected function for ' + key + '. Received ' + typeof fn);
          }
          if (!root.handlers[tag]) {
            root.handlers[tag] = {};
          }
          root.handlers[tag][key] = fn;
        }
      };
      root.handler(call);
    }
    return root.handlers[tag];
  }

  onData: {
    'use strict';
    var handler = getHandler(tag);
    if (!handler) {
      console.warn('Received DataEnd for unknown tag: ' + tag);
      return;
    }
    var fn = handler.data;
    if (typeof fn !== 'function') {
      console.warn('Handler is not registered for data event.');
      return;
    }
    fn(new root.readType(data));
  }

  onError: {
    'use strict';
    var handler = getHandler(tag);
    if (!handler) {
      console.warn('Received DataEnd for unknown tag: ' + tag);
      return;
    }
    var fn = handler.error;
    if (typeof fn !== 'function') {
      console.warn('Handler is not registered for data event.');
      return;
    }
    fn(err);
  }
}
