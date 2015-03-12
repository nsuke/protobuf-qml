.pragma library

var _proc = {
  get async() {
    if (!this._async) {
      this._component = Qt.createComponent('Async.qml');
      this._async = this._component.createObject();
    }
    return this._async;
  },
};

var Message = function(descriptor) {
  this._descriptor = descriptor;
  this._fields = [];
  this.parseFrom = function(input, onSuccess, onError) {
    _proc.async.parse(
      this._descriptor, this._fields, input, onSuccess, onError);
  };
  this.serializeTo = function(output, onSuccess, onError) {
    _proc.async.serialize(
      this._descriptor, this._fields, output, onSuccess, onError);
  };
};
