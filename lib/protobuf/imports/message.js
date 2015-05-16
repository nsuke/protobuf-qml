'use strict';
Qt.include('call.js');

var createMessageType = (function() {
  var MessageBase = (function() {
    var constructor = function(descriptor) {
      this._descriptor = descriptor;

      this.serializeTo = function(output, cb) {
        try {
          output.descriptor = this._descriptor;
          var call = new UnaryMethod(output);
          call.call(this, function(data, err) {
            if (data) {
              console.warn('Serialize callback received data object unexpectedly and ignored it.');
            }
            cb(err);
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

    type.parseFrom = function(input, cb) {
      try {
        input.descriptor = desc;
        var call = new UnaryMethod(input);
        call.call(undefined, function(data, err) {
          if (err) {
            cb(undefined, err);
          } else {
            var obj = new type();
            obj._mergeFromRawArray(data);
            cb(obj);
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