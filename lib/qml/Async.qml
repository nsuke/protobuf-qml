import QtQuick 2.2
import Protobuf 1.0

Item {
  QtObject {
    id: detail

    function handlerStorage() {
      var that = this;
      this.handlers = [];

      this.index = {
        _i: 0,
        get next() {
          for (var i = this._i;
          i < that.handlers.length && typeof that.handlers[i] != 'undefined'; ++i) {
          }
          this._i = i;
          return this._i;
        },
        set next(v) {
          this._i = v;
        },
      };
      
      this.addHandler = function(cb) {
        var i = that.index.next;
        var entry = {
          timestamp: Date.now(),
          'handler': cb,
        };
        that.handlers[i] = entry;
        return i;
      };
    }

    property var parseHandlers: new handlerStorage()
    property var serializeHandlers: new handlerStorage()
    property var serializeArrayHandlers: new handlerStorage()

    property var processor: AsyncProcessor {
      onParsed: detail.handle(detail.parseHandlers, id, fields, err);
      onSerialized: detail.handle(detail.serializeHandlers, id, err);
      onSerializedArray: detail.handle(detail.serializeArrayHandlers, id, array, err);
    }

    function handle(storage, id, data, err) {
      'use strict';
      var entry = storage.handlers[id];
      var cb = entry['handler'];
      if (cb) {
        if (typeof cb != 'function') {
          console.error('Callback is not function : (ID: ' + id + ', TYPE: ' + target + ')');
        } else {
          cb(data, err);
          delete storage.handlers[id];
          storage.index.next = id;
        }
      } else {
        if (!optional) {
          console.error('Callback not found for ID : ' + id);
        }
        console.log(data);
      }
    }

    function executeAsync(storage, f, cb, args) {
      'use strict';
      var id = storage.addHandler(cb);
      args.push(id);
      processor[f].apply(processor, args);
    }
  }

  function parse(input, descriptor, cb) {
    'use strict';
    detail.executeAsync(detail.parseHandlers, 'parse',
        cb, [input, descriptor]);
  }

  function parseArray(input, descriptor, cb) {
    'use strict';
    detail.executeAsync(detail.parseHandlers, 'parseArray',
        cb, [input, descriptor]);
  }

  function serialize(output, descriptor, message, cb) {
    'use strict';
    console.assert(output);
    console.assert(descriptor);
    detail.executeAsync(detail.serializeHandlers, 'serialize',
        cb, [output, descriptor, message[0], message[1]]);
  }

  function serializeArray(descriptor, message, cb) {
    'use strict';
    detail.executeAsync(detail.serializeArrayHandlers, 'serializeArray',
        cb, [descriptor, message]);
  }
}
