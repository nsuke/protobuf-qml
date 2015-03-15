.pragma library
.import QtQuick 2.4 as QtQuick
'use strict';

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

var Message = function(descriptor) {
  this._descriptor = descriptor;

  this.serializeTo = function(output, onSuccess, onError) {
    _proc.async.serialize(output, 
      this._descriptor, this._raw, onSuccess, onError);
  };

  this.serializeToArray = function(onSuccess, onError) {
    _proc.async.serializeArray(
      this._descriptor, this._raw, onSuccess, onError);
  };
};

var createMessageType = function(type, desc) {
  type.prototype = new Message(desc);

  type.parseFrom = function(input, onSuccess, onError) {
    _proc.async.parse(input, desc, function(fields) {
      var obj = new type();
      obj._mergeFromRawArray(fields);
      onSuccess(obj);
    }, onError);
  };

  type.parseFromArray = function(input, onSuccess, onError) {
    _proc.async.parseArray(input, desc, function(fields) {
      var obj = new type();
      obj._mergeFromRawArray(fields);
      onSuccess(obj);
    }, onError);
  };
};
