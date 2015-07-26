.pragma library
'use strict';

var createMessageType = (function() {
  // Use "true" async callback using thread pool.
  var async = true;

  var MessageBase = (function() {
    var Base = function(descriptor) {
      this._descriptor = descriptor;
    };

    Base.prototype.serialize = function(cb) {
      if (async || typeof cb == 'undefined') {
        return this._descriptor.v4().serialize(this._raw, cb);
      } else {
        try {
          cb(null, this._descriptor.v4().serialize(this._raw));
        } catch (err) {
          cb(err);
        }
      }
    };

    Base.prototype.serializeTo = function(output, cb) {
      try {
        output.serialize(this.constructor, this, cb);
      } catch (err) {
        console.log('Serialize Error !');
        console.error(err);
        console.error(err.stack);
        cb && cb(err);
      }
    };

    Base.prototype.toString = function() {
      return String(this._raw);
    };

    Base.prototype._ensureLength = function(array, len) {
      if (!array.BYTES_PER_ELEMENT) {
        console.trace();
        throw new TypeError('Not a typed array.');
      }
      if (typeof len == 'undefined') {
        len = array.length + 1;
      }
      var blen = len * array.BYTES_PER_ELEMENT;
      if (array.buffer.byteLength < blen) {
        // Beware, biggest performance hit here.
        // console.warn('RESIZING FROM ' + array.length + ' TO ' + len);
        var newLen = Math.max(array.buffer.byteLength * 2, blen);
        var buffer = new ArrayBuffer(newLen);
        var newArray = new array.constructor(buffer, 0, len);
        newArray.set(array);
        return newArray;
      }
      if (array.length < len) {
        return new array.constructor(array.buffer, 0, len);
      }
    };

    Base.prototype._maybeConvertToMessage = function(Type, obj) {
      return obj instanceof Type ? obj : new Type(obj);
    }

    Object.defineProperties(Base, {
      FIELD: {value: 0},
      ONEOF: {value: 1},
    });
    return Base;
  })();

  var createMessageType = function(Message, descriptor) {
    Message.prototype = new MessageBase(descriptor);
    Message.prototype.constructor = Message;

    Message.descriptor = descriptor;

    Message.parse = function(buffer, cb) {
      if (typeof cb == 'undefined') {
        return new Message(descriptor.v4().parse(buffer));
      } else if (async) {
        return descriptor.v4().parse(buffer, function(err, raw) {
          cb(err, new Message(raw));
        });
      } else {
        try {
          cb(null, new Message(descriptor.v4().parse(buffer)));
        } catch (err) {
          cb(err);
        }
      }
    };

    Message.parseFrom = function(input, cb) {
      try {
        input.parse(Message, function(err, data) {
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
