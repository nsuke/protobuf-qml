import QtQuick 2.4
import Protobuf 1.0

Item {
  QtObject {
    id: detail

    readonly property var signalTypes: ({
      'ERROR': 0,
      'PARSED': 1,
      'SERIALIZED': 2,
      'SERIALIZED_ARRAY': 3,
    })

    property var index: ({
      _i: 0,
      get next() {
        for (var i = this._i;
            i < handlers.length && typeof handlers[i] != 'undefined'; ++i) {
        }
        this._i = i;
        return this._i;
      },
      set next(v) {
        this._i = v;
      },
    })

    property var handlers: []

    property var processor: AsyncProcessor {
      onError: detail.handle(detail.signalTypes.ERROR, id, error, true);
      onParsed: detail.handle(detail.signalTypes.PARSED, id, fields);
      onSerialized: detail.handle(detail.signalTypes.SERIALIZED, id);
      onSerializedArray: detail.handle(detail.signalTypes.SERIALIZED_ARRAY, id, array);
    }

    function handle(target, id, data, optional) {
      'use strict';
      if (typeof optional == 'undefined') {
        optional = false;
      }
      var cb = handlers[id];
      if (cb) {
        cb[target](data);
        delete handlers[id];
        index.next = id;
      } else if (!optional) {
        console.warn('Callback not found for ID : ' + id);
      }
    }

    function executeAsync(type, f, onSuccess, onError, args) {
      'use strict';
      // console.log('Executing ' + f + ' with args ' + args);
      var id = detail.index.next;
      var handler = {
        timestamp: Date.now(),
      };
      handler[signalTypes.ERROR] = onError;
      handler[type] = onSuccess;
      handlers[id] = handler;
      args.push(id);
      processor[f].apply(processor, args);
    }
  }

  function parse(input, descriptor, onSuccess, onError) {
    'use strict';
    detail.executeAsync(detail.signalTypes.PARSED, 'parse',
        onSuccess, onError, [input, descriptor]);
  }

  function parseArray(input, descriptor, onSuccess, onError) {
    'use strict';
    detail.executeAsync(detail.signalTypes.PARSED, 'parseArray',
        onSuccess, onError, [input, descriptor]);
  }

  function serialize(output, descriptor, message, onSuccess, onError) {
    'use strict';
    console.assert(output);
    console.assert(descriptor);
    detail.executeAsync(detail.signalTypes.SERIALIZED, 'serialize',
        onSuccess, onError, [output, descriptor, message]);
  }

  function serializeArray(descriptor, message, onSuccess, onError) {
    'use strict';
    detail.executeAsync(detail.signalTypes.SERIALIZED_ARRAY, 'serializeArray',
        onSuccess, onError, [descriptor, message]);
  }
}
