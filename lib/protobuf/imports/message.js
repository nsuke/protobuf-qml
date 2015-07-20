.pragma library
'use strict';

var createMessageType = (function() {
  // Use "true" async callback using thread pool.
  var async = true;

  var MessageBase = (function() {
    var constructor = function(descriptor) {
      this._descriptor = descriptor;

      this.serialize = function(cb) {
        if (async || typeof cb == 'undefined') {
          return descriptor.v4().serialize(this._raw, cb);
        } else {
          try {
            cb(null, descriptor.v4().serialize(this._raw));
          } catch (err) {
            cb(err);
          }
        }
      };

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

  var createMessageType = function(type, descriptor) {
    type.prototype = new MessageBase(descriptor);
    type.prototype.type = type;

    type.descriptor = descriptor;

    type.parse = function(buffer, cb) {
      if (typeof cb == 'undefined') {
        return new type(descriptor.v4().parse(buffer));
      } else if (async) {
        return descriptor.v4().parse(buffer, function(err, raw) {
          cb(err, new type(raw));
        });
      } else {
        try {
          cb(null, new type(descriptor.v4().parse(buffer)));
        } catch (err) {
          cb(err);
        }
      }
    };

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
