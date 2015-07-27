import QtQuick 2.5
import Protobuf 1.0 as PB

PB.ServerReaderWriterMethodHolder {
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
          var msg = response instanceof root.writeType ? response : new root.writeType(response);
          root.respond(tag, msg._raw);
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

  onDataEnd: {
    'use strict';
    var handler = handlers[tag];
    if (!handler) {
      console.warn('Received DataEnd for unknown tag: ' + tag);
      return;
    }
    var fn = handler.end;
    if (typeof fn !== 'function') {
      console.warn('Handler is not registered for end event.');
      return;
    }
    fn();
  }

  onData: {
    'use strict';
    var handler = getHandler(tag);
    if (!handler) {
      console.warn('Received Data for unknown tag: ' + tag);
      return;
    }
    var fn = handler.data;
    if (typeof fn !== 'function') {
      console.warn('Handler is not registered for data event.');
      return;
    }
    var msg = data instanceof root.readType ? data : new root.readType(data);
    fn(msg);
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
