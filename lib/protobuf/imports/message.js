.pragma library
'use strict';

var createMessageType = (function() {
  var MessageBase = (function() {
    var constructor = function(descriptor) {
      this._descriptor = descriptor;

      this.serializeTo = function(output, cb) {
        try {
          output.serialize(this.type, this, cb);
        } catch (err) {
          console.log('Serialize Error !');
          console.error(err);
          console.error(err.stack);
          cb && cb(err);
        }
      };

      this.toString = function() {
        return String(this._raw);
      }
    };
    Object.defineProperties(constructor, {
      FIELD: {value: 0},
      ONEOF: {value: 1},
    });
    return constructor;
  })();

  var createMessageType = function(type, desc) {
    type.prototype = new MessageBase(desc);
    type.prototype.type = type;

    type.descriptor = desc;

    type.parseFrom = function(input, cb) {
      try {
        input.parse(type, function(err, data) {
          if (err) {
            cb(err);
          } else {
            cb(null, data);
          }
        });
      } catch (err) {
        console.log('Parse Error !');
        console.error(err);
        console.error(err.stack);
        cb && cb(err);
      }
    };
  };

  return createMessageType;
})();
