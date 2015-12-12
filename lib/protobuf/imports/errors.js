.pragma library
'use strict';

var _errors = [];

/*
As of writing, Error properties are made read-only, making it impossible to
create custom error types.
For Qt 5.5, it was possible to capture stacktrace like this. It's disabled
because there's no way to determine Qt version from scripts at run-time.

function _registerErrorType(name, code) {
  var type = function() {
    var err = Error.apply(this, arguments);
    this.message = err.message;
    this.stack = err.stack;
  };
  type.prototype = Object.create(Error.prototype);
  type.prototype.constructor = type;
  type.prototype.name = name;
  type.prototype.code = code;
  _errors[code] = type;
  return type;
}
*/

function _registerErrorType(name, code) {
  var type = function(message) {
    this.message = message;
  };
  type.prototype.constructor = type;
  type.prototype.name = name;
  type.prototype.code = code;
  _errors[code] = type;
  return type;
}

function forStatusCode(code) {
  return _errors[code];
}

// Sync with protobuf::qml::StatusCode enum
var Cancelled = _registerErrorType('Cancelled', 1);
var Unknown = _registerErrorType('Unknown', 2);
var InvalidArgument = _registerErrorType('InvalidArgument', 3);
var DeadlineExceeded = _registerErrorType('DeadlineExceeded', 4);
var NotFound = _registerErrorType('NotFound', 5);
var AlreadyExists = _registerErrorType('AlreadyExists', 6);
var PermissionDenied = _registerErrorType('PermissionDenied', 7);
var Unauthenticated = _registerErrorType('Unauthenticated', 16);
var ResourceExhausted = _registerErrorType('ResourceExhausted', 8);
var FailedPrecondition = _registerErrorType('FailedPrecondition', 9);
var Aborted = _registerErrorType('Aborted', 10);
var OutOfRange = _registerErrorType('OutOfRange', 11);
var Unimplemented = _registerErrorType('Unimplemented', 12);
var Internal = _registerErrorType('Internal', 13);
var Unavailable = _registerErrorType('Unavailable', 14);
var DataLoss = _registerErrorType('DataLoss', 15);
var DoNotUse = _registerErrorType('DoNotUse', -1);
