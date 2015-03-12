import QtQuick 2.4
import Protobuf 1.0

QtObject {
  QtObject {
    id: detail
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
    property var processor: Protobuf.AsyncProcessor {
      onSuccess: handle(1, id, data);
      onError: handle(0, id, error);
    }
    function handle(target, id, data) {
      var cb = handlers[id];
      if (!cb) {
        console.warn('Callback not found for ID : ' + id);
      } else {
        cb[target](data);
        delete handlers[id];
        index.next = id;
      }
    }

    function execute(f, io, descriptor, message, onSuccess, onError) {
      var id = index.next;
      handlers[id] = [
        onError,
        onSuccess,
        Date.now(),
      ];
      if (Protobuf.QByteArrayUtil.isInstance(output) {
        f = f + 'Array';
      }
      processor[f](io, desc, message, id)
    }
  }

  function serialize(output, descriptor, message, onSuccess, onError) {
    detail.execute(
      'serialize', output, descriptor, message, onSuccess, onError);
  }

  function parse(input, descriptor, message, onSuccess, onError) {
    detail.execute('parse', input, descriptor, message, onSuccess, onError);
  }
}
