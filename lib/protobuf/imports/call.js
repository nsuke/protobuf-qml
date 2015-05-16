'use strict';

var MethodBase = function(impl) {
  if (!impl) {
    throw new Error('Implementation object is invalid.');
  }
  this._impl = impl;
}

MethodBase.prototype._registerSingleRead = function(cb) {
  console.log('register');
  var tag = this._impl.getFreeTag();
  if (!tag) {
    throw new Error('Unable to open call session.');
  }
  var errorCallback = function(t, err) {
    console.log('error');
    if (t === tag) {
      cb && cb(undefined, err || 'Unknown error');
      disconnect();
    }
  };
  var dataCallback = function(t, data) {
    console.log('data');
    if (t === tag) {
      console.log('data hit');
      cb && cb(data);
      disconnect();
    }
  };
  var that = this;
  var disconnect = function() {
    console.log('disc');
    that._impl.error.disconnect(errorCallback);
    that._impl.data.disconnect(dataCallback);
  };
  this._impl.error.connect(errorCallback);
  this._impl.data.connect(dataCallback);
  return tag;
};

MethodBase.prototype._registerMultiRead = function(cb) {
  var tag = this._registerMultiRead(cb);
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
  };
  var dataEndCallback = function(t) {
    if (t === tag) {
      cb && cb(undefined, undefined, true);
      disconnect();
    }
  };
  var disconnect = function() {
    this._impl.error.disconnect(errorCallback);
    this._impl.read.disconnect(dataCallback);
    this._impl.readEnd.disconnect(dataEndCallback);
  };
  this._impl.error.connect(errorCallback);
  this._impl.data.connect(dataCallback);
  this._impl.dataEnd.connect(dataEndCallback);
  return tag;
};

MethodBase.prototype._singleWrite = function(tag, msg) {
  msg ? this._impl.write(tag, msg._raw)
    : this._impl.read(tag);
}

MethodBase.prototype._multiWrite = function(tag) {
  this._impl.read(tag);
  return {
    write: function(msg) {
      if (!tag) {
        throw new Error('Writer session is closed.');
      }
      this._impl.write(tag, msg._raw);
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
