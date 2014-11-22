var Message = function(){};
Message.prototype = function() {
  this.parseFrom = function(input) {
    var context = input.createContext();
    if (!context) return false;
    this.value1 = Format.parseInt(input);
    delete context;
    return true;
  };
  this.serializeTo = function(output) {
    var context = output.createContext();
    if (!context) return false;
    Format.serializeInt(context, this.value1);
    delete context;
    return true;
  };
};
