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
        try {
          var tag = output.getFreeTag();
          output.descriptor = this._descriptor;
          var e = function(t, err) {
            console.log('err : ' + t + ' : ' + err);
            if (t === tag) {
              if (!err) {
                console.warn('Serialize error callback recieved empty eror.');
              }
              cb(err || 'Unknown error');
              disc();
            }
          };
          var d = function(t, data) {
            if (t === tag) {
              if (data) {
                console.warn('Serialize callback received data unexpectedly and ignored');
              }
              cb();
              disc();
            }
          };
          var disc = function() {
            output.error.disconnect(e);
            output.data.disconnect(d);
          };
          output.error.connect(e);
          output.data.connect(d);
          output.write(tag, this._raw);
        } catch (err) {
          console.error(err);
          cb && cb(err);
        }
      };

      this.__serializeTo = function(output, cb) {
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
      try {
        var tag = input.getFreeTag();
        var e = function(t, err) {
          console.log('err : ' + t + ' : ' + err);
          if (t === tag) {
            if (!err) {
              console.warn('Serialize error callback recieved empty eror.');
            }
            cb(undefined, err || 'Unknown error');
            disc();
          }
        };
        var d = function(t, data) {
          if (t === tag) {
            var obj = new type();
            obj._mergeFromRawArray(data);
            cb(obj);
            disc();
          }
        };
        var disc = function() {
          input.error.disconnect(e);
          input.data.disconnect(d);
        };
        input.error.connect(e);
        input.data.connect(d);
        input.descriptor = desc;
        input.read(tag);
      } catch (err) {
        console.error(err);
        cb && cb(undefined, err);
      }
    };

    type.__parseFrom = function(input, cb) {
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
