.pragma library
'use strict';

var MethodBase = function(impl) {
  if (!impl) {
    throw new Error('Implementation object is invalid.');
  }
  this._impl = impl;
  this._initialized = false;
  this._impl.methodNameChanged.connect(this.deinit);
  this._impl.readDescriptorChanged.connect(this.deinit);
  this._impl.writeDescriptorChanged.connect(this.deinit);
};

MethodBase.prototype.deinit = function() {
  that._initialized = false;
};

MethodBase.prototype._ensure_init = function() {
  if (!this._initialized) {
    this._initialized = this._impl.init();
    if (!this._initialized) {
      throw new Error('Unable to initialize method.');
    }
  }
};

MethodBase.prototype._registerRead = function(cb, oneshot) {
  this._ensure_init();
  var tag = this._impl.getFreeTag();
  if (!tag) {
    throw new Error('Unable to open writer session.');
  }
  var errorCallback = function(t, err) {
    if (t === tag) {
      cb && cb(undefined, err || 'Unknown error');
      disconnect();
    }
  };
  var dataCallback = function(t, data) {
    if (t === tag) {
      cb && cb(data);
    }
    if (oneshot) {
      disconnect();
    }
  };
  var dataEndCallback = function(t) {
    if (t === tag) {
      if (oneshot) {
        // For now, consider data end for oneshot an error.
        cb && cb(undefined, 'Received data end.');
      } else {
        cb && cb(undefined, undefined, true);
      }
      disconnect();
    }
  };
  var that = this;
  var disconnect = function() {
    that._impl.error.disconnect(errorCallback);
    that._impl.data.disconnect(dataCallback);
    that._impl.dataEnd.disconnect(dataEndCallback);
  };
  this._impl.error.connect(errorCallback);
  this._impl.data.connect(dataCallback);
  this._impl.dataEnd.connect(dataEndCallback);
  return tag;
};

MethodBase.prototype._registerSingleRead = function(cb) {
  return this._registerRead(cb, true);
};

MethodBase.prototype._registerMultiRead = function(cb) {
  return this._registerRead(cb, false);
};

MethodBase.prototype._singleWrite = function(tag, msg) {
  this._ensure_init();
  this._impl.write(tag, msg);
  this._impl.writeEnd(tag);
}

MethodBase.prototype._multiWrite = function(tag) {
  this._ensure_init();
  return {
    write: function(msg) {
      if (!tag) {
        throw new Error('Writer session is closed.');
      }
      this._impl.write(tag, msg);
    },

    writeEnd: function() {
      if (!tag) {
        throw new Error('Writer session is closed.');
      }
      this._impl.writeEnd(tag);
      tag = 0;
    },
  };
}

var ReaderWriterMethod = function(impl) { MethodBase.call(this, impl); };
ReaderWriterMethod.prototype = Object.create(MethodBase.prototype);
ReaderWriterMethod.prototype.call = function(cb) {
  return this._multiWrite(this._registerMultiRead(cb));
};
var readerWriterCall = function(impl, cb) {
  var method = new ReaderWriterMethod(impl);
  return method.call(cb);
}

var WriterMethod = function(impl) { MethodBase.call(this, impl); };
WriterMethod.prototype = Object.create(MethodBase.prototype);
WriterMethod.prototype.call = function(cb) {
  return this._multiWrite(this._registerSingleRead(cb));
};

var ReaderMethod = function(impl) { MethodBase.call(this, impl); };
ReaderMethod.prototype = Object.create(MethodBase.prototype);
ReaderMethod.prototype.call = function(msg, cb) {
  this._singleWrite(this._registerMultiRead(cb), msg);
};

var UnaryMethod = function(impl) { MethodBase.call(this, impl); };
UnaryMethod.prototype = Object.create(MethodBase.prototype);
UnaryMethod.prototype.call = function(msg, cb) {
  this._singleWrite(this._registerSingleRead(cb), msg);
};
var unaryCall = function(impl, msg, cb) {
  var method = new UnaryMethod(impl);
  method.call(msg, cb);
};
