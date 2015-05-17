.pragma library
.import 'call.js' as Call
'use strict';

var createMessageType = (function() {
  var MessageBase = (function() {
    var constructor = function(descriptor) {
      this._descriptor = descriptor;

      this.serializeTo = function(output, cb) {
        try {
          output.writeDescriptor = this._descriptor;
          Call.unaryCall(output, this._raw, function(data, err) {
            if (data) {
              console.warn('Serialize callback received data object unexpectedly and ignored it.');
            }
            cb && cb(err);
          });
        } catch (err) {
          console.log('Serialize Error !');
          console.error(err);
          console.error(err.stack);
          cb && cb(err);
        }
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

    type.descriptor = desc;

    type.parseFrom = function(input, cb) {
      try {
        input.readDescriptor = desc;
        Call.unaryCall(input, undefined, function(data, err) {
          if (err) {
            cb(undefined, err);
          } else {
            cb(new type(data));
          }
        });
      } catch (err) {
        console.log('Parse Error !');
        console.error(err);
        console.error(err.stack);
        cb && cb(undefined, err);
      }
    };
  };

  return createMessageType;
})();
