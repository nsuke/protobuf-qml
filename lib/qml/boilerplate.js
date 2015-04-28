.pragma library
.import QtQuick 2.2 as QtQuick
'use strict';

var createMessageType = (function() {
  var _proc = {
    get async() {
      if (!this._async) {
        this._component = Qt.createComponent('Async.qml');
        while (this._component.status == QtQuick.Component.Loading) {
        }
        if (this._component.status != QtQuick.Component.Ready) {
          console.error('Failed to initialize async component.');
        } else {
          this._async = this._component.createObject();
        }
      }
      if (!this._async)
        throw new Error('Async execution engine is not available.');
      return this._async;
    },
  };

  var MessageBase = (function() {
    var constructor = function(descriptor) {
      this._descriptor = descriptor;

      this.serializeTo = function(output, cb) {
        _proc.async.serialize(output,
                              this._descriptor, this._raw, cb);
      };

      this.serializeToArray = function(cb) {
        _proc.async.serializeArray(
          this._descriptor, this._raw, cb);
      };
    };
    Object.defineProperties(constructor, {
      FIELD: {value: 0},
      ONEOF: {value: 1},
    });
    return constructor;
  })();

  var createMessageType = function(type, desc) {
    type.prototype = new MessageBase(desc);

    type.parseFrom = function(input, cb) {
      _proc.async.parse(input, desc, function(fields, err) {
        if (err) {
          cb(null, err);
        } else {
          var obj = new type();
          obj._mergeFromRawArray(fields);
          cb(obj);
        }
      });
    };

    type.parseFromArray = function(input, cb) {
      _proc.async.parseArray(input, desc, function(fields, err) {
        if (err) {
          cb(null, err);
        } else {
          var obj = new type();
          obj._mergeFromRawArray(fields);
          cb(obj);
        }
      });
    };
  };

  return createMessageType;
})();
