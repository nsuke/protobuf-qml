#include "protobuf/qml/field_generator.h"
#include "protobuf/qml/enum_generator.h"
#include "protobuf/qml/util.h"
#include "protobuf/qml/compiler_util.h"
#include "protobuf/qml/compiler_common.h"

namespace protobuf {
namespace qml {

using namespace google::protobuf;

FieldGenerator::FieldGenerator(const FieldDescriptor* t)
    : t_(t),
      is_message_(t_ ? t_->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE
                     : false),
      oneof_(t_ ? t_->containing_oneof() : nullptr),
      camel_name_(t_ ? camelize(t_->name()) : "") {
  capital_name_ = capitalizeFirstLetter(camel_name_);
  if (t_) {
    variables_ = {
        {"i", "  "},  // indent
        {"type", "type"},
        {"default", defaultValue()},
        {"name", camel_name_},
        {"capital_name", capital_name_},
        {"all_capital_name", capitalizeAll(t_->name())},
        {"index", std::to_string(t_->index())},
    };
    if (t_->message_type()) {
      auto scope = t_->file() == t_->message_type()->file()
                       ? ""
                       : generateImportName(t_->message_type()->file()) + ".";
      variables_.insert(std::make_pair("message_scope", scope));
      variables_.insert(
          std::make_pair("message_type", generateLongName(t_->message_type())));
    }
    if (oneof_) {
      auto oneof_camel = camelize(oneof_->name());
      auto oneof_capital = capitalizeFirstLetter(oneof_camel);
      variables_.insert(std::make_pair("oneof_camel", std::move(oneof_camel)));
      variables_.insert(
          std::make_pair("oneof_capital", std::move(oneof_capital)));
      variables_.insert(
          std::make_pair("oneof_all_capital", capitalizeAll(oneof_->name())));
      variables_.insert(
          std::make_pair("oneof_index", std::to_string(oneof_->index())));
    }
  }

  if (t_->cpp_type() == FieldDescriptor::CPPTYPE_INT32 ||
      t_->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
    is_typed_array_ = true;
    variables_.insert(std::make_pair("typed_array", "Int32Array"));
  } else if (t_->cpp_type() == FieldDescriptor::CPPTYPE_UINT32) {
    is_typed_array_ = true;
    variables_.insert(std::make_pair("typed_array", "Uint32Array"));
  } else if (t_->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT) {
    is_typed_array_ = true;
    variables_.insert(std::make_pair("typed_array", "Float32Array"));
  } else if (t_->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE) {
    is_typed_array_ = true;
    variables_.insert(std::make_pair("typed_array", "Float64Array"));
  } else if (t_->cpp_type() == FieldDescriptor::CPPTYPE_BOOL) {
    is_typed_array_ = true;
    variables_.insert(std::make_pair("typed_array", "Int8Array"));
  }
}

const char* boolToString(bool value) {
  return value ? "true" : "false";
}

std::string FieldGenerator::defaultValue() {
  switch (t_->cpp_type()) {
#define PRINT_DEFAULT_VALUE(type1, type2, convert) \
  case FieldDescriptor::CPPTYPE_##type1:           \
    return convert(t_->default_value_##type2());
    PRINT_DEFAULT_VALUE(INT32, int32, std::to_string);
    PRINT_DEFAULT_VALUE(INT64, int64, std::to_string);
    PRINT_DEFAULT_VALUE(UINT32, uint32, std::to_string);
    PRINT_DEFAULT_VALUE(UINT64, uint64, std::to_string);
    PRINT_DEFAULT_VALUE(FLOAT, float, std::to_string);
    PRINT_DEFAULT_VALUE(DOUBLE, double, std::to_string);
    PRINT_DEFAULT_VALUE(BOOL, bool, boolToString);
    PRINT_DEFAULT_VALUE(ENUM, enum, generateLongName);
#undef PRINT_DEFAULT_VALUE

    case FieldDescriptor::CPPTYPE_STRING:
      if (t_->type() == FieldDescriptor::TYPE_BYTES) {
        return " new ArrayBuffer()";
      } else {
        return "'" + t_->default_value_string() + "'";
      }
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "{}";
    default:
      PBQML_ASSERT(false);
      return "";
  }
}

void FieldGenerator::generateInit(io::Printer& p) {
  if (t_->is_repeated()) {
    if (t_->message_type()) {
      p.Print(variables_, "$i$this._$name$ = new Array();\n");
    }
    if (is_typed_array_) {
      p.Print(variables_,
              "$i$var buffer = new ArrayBuffer(32);\n"
              "$i$this._$name$ = new $typed_array$(buffer, 0, 0);\n"
              "$i$this._raw[FIELD][$index$] = this._$name$;\n");
    } else if (t_->message_type()) {
      p.Print(variables_, "$i$this._raw[FIELD][$index$] = new Array();\n");
    } else {
      p.Print(variables_, "$i$this._raw[FIELD][$index$] = new Array();\n");
    }
    p.Print(variables_,
            "$i$if (values && values.$name$ && values.$name$ instanceof "
            "Array) {\n"
            "$i$  this.$name$(values.$name$);\n"
            "$i$}\n");
  }
}

void FieldGenerator::generateMerge(io::Printer& p, const std::string& arg) {
  auto v = variables_;
  v.insert(std::make_pair("arg", arg));
  if (t_->is_repeated() && t_->message_type()) {
    p.Print(v,
            "$i$if ($arg$[FIELD][$index$] && $arg$[FIELD][$index$] "
            "instanceof Array) {\n"
            "$i$  for (var i in $arg$[FIELD][$index$]) {\n"
            "$i$    if (typeof this._$name$[i] == 'undefined') {\n"
            "$i$      var msg = new $message_scope$$message_type$();\n"
            "$i$      this._$name$[i] = msg;\n"
            "$i$      this._raw[FIELD][$index$][i] = msg._raw;\n"
            "$i$    }\n"
            "$i$    "
            "this._$name$[i]._mergeFromRawArray($arg$[FIELD][$index$][i]);\n"
            "$i$  }\n"
            "$i$}\n");
  } else if (t_->is_repeated() && is_typed_array_) {
    p.Print(v,
            "$i$if ($arg$[FIELD][$index$] && ($arg$[FIELD][$index$] "
            "instanceof Array || $arg$[FIELD][$index$] instanceof "
            "$typed_array$) || $arg$[FIELD][$index$] instanceof ArrayBuffer) "
            "{\n"
            "$i$  this.$name$($arg$[FIELD][$index$]);\n"
            "$i$}\n");
  } else if (t_->is_repeated()) {
    p.Print(v,
            "$i$if ($arg$[FIELD][$index$] && $arg$[FIELD][$index$] instanceof "
            "Array) {\n"
            "$i$  this.$name$($arg$[FIELD][$index$]);\n"
            "$i$}\n");
  } else if (t_->message_type()) {
    auto v = variables_;
    v.insert(std::make_pair("arg", arg));
    p.Print(v,
            "$i$if (typeof this.$name$ == 'undefined') {\n"
            "$i$  this.$name$ = {};\n"
            "$i$}\n"
            "$i$this._$name$._mergeFromRawArray($arg$[FIELD][$index$]);\n");
  } else {
    p.Print(v, "$i$this.set$capital_name$($arg$[FIELD][$index$]);\n");
  }
}

void FieldGenerator::messageAssertLength(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "$i$console.assert(this._$name$.length == "
          "this._raw[FIELD][$index$].length);\n");
}

void genAdd(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "$i$$type$.prototype.add$capital_name$ = function(value) {\n"
          "$i$  if (typeof value == 'undefined') {\n"
          "$i$    throw new TypeError('Cannot add undefined.');\n"
          "$i$  }\n");

  if (is_message_) {
    p.Print(variables_,
            "$i$  var msg = "
            "this._maybeConvertToMessage($message_scope$$message_type$, "
            "value);\n"
            "$i$  this._$name$.push(msg);\n"
            "$i$  this._raw[FIELD][$index$].push(msg._raw);\n");
    messageAssertLength(p);
  } else if (is_typed_array_) {
    p.Print(variables_,
            "$i$  var newArray = this._ensureLength(this._$name$);\n"
            "$i$  if (newArray) {\n"
            "$i$    this._$name$ = newArray;\n"
            "$i$    this._raw[FIELD][$index$] = this._$name$;\n"
            "$i$  }\n"
            "$i$  this._$name$[this._$name$.length - 1] = value;\n");
  } else {
    p.Print(variables_, "$i$  this._raw[FIELD][$index$].push(value);\n");
  }

  p.Print(variables_, "$i$};\n");
}

void FieldGenerator::generateRepeatedProperty(
    google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "$i$  // Replacement setter\n"
          "$i$$type$.prototype.set$capital_name$ = function(values) {\n");
  if (is_typed_array_) {
    p.Print(variables_,
            "$i$  if (values instanceof $typed_array$ || values "
            "instanceof Array "
            "|| values instanceof ArrayBuffer) {\n"
            "$i$    if (!(values instanceof Array) && !values.name) {\n"
            "$i$      values = new $typed_array$(values);;\n"
            "$i$    }\n"
            "$i$    var newArray = this._ensureLength(this._$name$, "
            "values.length);\n"
            "$i$    if (newArray) {\n"
            "$i$      this._$name$ = newArray;\n"
            "$i$      this._raw[FIELD][$index$] = this._$name$;\n"
            "$i$    }\n"
            "$i$    this._$name$.set(values);\n"
            "$i$  } else {\n"
            "$i$    throw new TypeError();\n"
            "$i$  }\n");
  } else {
    p.Print(variables_,
            "$i$  if (!(values instanceof Array)) {\n"
            "$i$    throw new TypeError();\n"
            "$i$  }\n");
  }
  if (is_message_) {
    p.Print(variables_,
            "$i$  this._raw[FIELD][$index$].length = values.length;\n"
            "$i$   this._$name$.length = values.length;\n"
            "$i$   for (var i in values) {\n"
            "$i$     var msg = new "
            "$message_scope$$message_type$(values[i]);\n"
            "$i$     this._$name$[i] = msg;\n"
            "$i$     this._raw[FIELD][$index$][i] = msg._raw;\n"
            "$i$   }\n");
  } else if (!is_typed_array_) {
    p.Print(variables_, "$i$    this._raw[FIELD][$index$] = values.slice();\n");
  }
  p.Print(variables_,
          "$i$};\n"
          "$i$// Single value setter\n"
          "$i$$type$.prototype.set$capital_name$At = function(index, value) "
          "{\n"
          "$i$  if (typeof index != 'number') {\n"
          "$i$    throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "$i$  }\n"
          "$i$  if(this.$name$Size < index) {\n"
          "$i$    throw new RangeError();\n"
          "$i$  }\n");
  if (is_message_) {
    p.Print(variables_,
            "$i$  var msg = "
            "this._maybeConvertToMessage($message_scope$$message_type$, "
            "value);\n"
            "$i$  this._$name$[index] = msg;\n"
            "$i$  this._raw[FIELD][$index$][index] = msg._raw;\n");
  } else if (is_typed_array_) {
    p.Print(variables_, "$i$  this._$name$[index] = value;\n");
  } else {
    p.Print(variables_, "$i$  this._raw[FIELD][$index$][index] = value;\n");
  }
  p.Print(variables_,
          "$i$};\n"
          "$i$// Getter\n"
          "$i$$type$.prototype.get$capital_name$At = function(index) {\n"
          "$i$  if (typeof index != 'number') {\n"
          "$i$    throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "$i$  }\n"
          "$i$  if(this._raw[FIELD][$index$].length < index) {\n"
          "$i$    throw new RangeError();\n"
          "$i$  }\n");
  if (is_message_ || is_typed_array_) {
    p.Print(variables_, "$i$  return this._$name$[index];\n");
  } else {
    p.Print(variables_, "$i$  return this._raw[FIELD][$index$][index];\n");
  }
  p.Print(variables_,
          "$i$};\n"
          "$i$$type$.prototype.get$capital_name$AsArray = function() {\n");
  if (is_message_) {
    p.Print(variables_, "$i$  return this._$name$.slice();\n");
  } else if (is_typed_array_) {
    p.Print(variables_,
            "$i$  var array = [];\n"
            "$i$  for (var i = 0; i < this._$name$.length; ++i) {\n"
            "$i$    array.push(this._$name$[i]);\n"
            "$i$  }\n"
            "$i$  return array;\n");
  } else {
    p.Print(variables_, "$i$  return this._raw[FIELD][$index$].slice();\n");
  }
  p.Print(variables_, "$i$};\n");

  p.Print(variables_,
          "$i$$type$.prototype.$name$ = function(indexOrValues, value) {\n"
          "$i$  if (typeof indexOrValues == 'undefined') {\n"
          "$i$    throw new TypeError('Not enough arguments');\n"
          "$i$  } else if (typeof indexOrValues == 'number') {\n"
          "$i$    if (typeof value == 'undefined') {\n"
          "$i$      return this.get$capital_name$At(indexOrValues);\n"
          "$i$    } else {\n"
          "$i$      this.set$capital_name$At(indexOrValues, value);\n"
          "$i$    }\n"
          "$i$  } else {\n"
          "$i$    this.set$capital_name$(indexOrValues);\n"
          "$i$  }\n"
          "$i$};\n"
          "$i$var $name$Count = function() {\n");

  if (is_typed_array_ || is_message_) {
    if (is_message_) messageAssertLength(p);
    p.Print(variables_, "$i$  return this._$name$.length;\n");
  } else {
    p.Print(variables_, "$i$  return this._raw[FIELD][$index$].length;\n");
  }

  p.Print(variables_,
          "$i$};\n"
          "$i$Object.defineProperties($type$.prototype, {\n"
          "$i$  $name$Count: { get: $name$Count },\n"
          "$i$  $name$Size: { get: $name$Count },\n"
          "$i$  $name$Length: { get: $name$Count },\n"
          "$i$});\n"
          "$i$$type$.prototype.get$capital_name$Count = $name$Count;\n"
          "$i$$type$.prototype.get$capital_name$Size = $name$Count;\n"
          "$i$$type$.prototype.get$capital_name$Length = $name$Count;\n"
          "$i$$type$.prototype.add$capital_name$ = function(value) {\n"
          "$i$  if (typeof value == 'undefined') {\n"
          "$i$    throw new TypeError('Cannot add undefined.');\n"
          "$i$  }\n");

  if (is_message_) {
    p.Print(variables_,
            "$i$  var msg = "
            "this._maybeConvertToMessage($message_scope$$message_type$, "
            "value);\n"
            "$i$  this._$name$.push(msg);\n"
            "$i$  this._raw[FIELD][$index$].push(msg._raw);\n");
    messageAssertLength(p);
  } else if (is_typed_array_) {
    p.Print(variables_,
            "$i$  var newArray = this._ensureLength(this._$name$);\n"
            "$i$  if (newArray) {\n"
            "$i$    this._$name$ = newArray;\n"
            "$i$    this._raw[FIELD][$index$] = this._$name$;\n"
            "$i$  }\n"
            "$i$  this._$name$[this._$name$.length - 1] = value;\n");
  } else {
    p.Print(variables_, "$i$  this._raw[FIELD][$index$].push(value);\n");
  }

  p.Print(variables_,
          "$i$};\n"
          "$i$$type$.prototype.remove$capital_name$ = function(index) {\n"
          "$i$  if (typeof index != 'number') {\n"
          "$i$    throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "$i$  }\n"
          "$i$  this._raw[FIELD][$index$].splice(index, 1);\n");

  if (is_message_) {
    p.Print(variables_, "$i$  this._$name$.splice(index, 1);\n");
    messageAssertLength(p);
  }

  p.Print(variables_,
          "$i$};\n"
          "$i$$type$.prototype.clear$capital_name$ = function() {\n"
          "$i$  this._raw[FIELD][$index$].length = 0;\n");

  if (is_message_) {
    p.Print(variables_, "$i$  this._$name$.length = 0;\n");
    messageAssertLength(p);
  }

  p.Print(variables_, "$i$};\n");
}

void FieldGenerator::genGet(google::protobuf::io::Printer& p) {
  if (is_message_) {
    p.Print(variables_, "$i$return this._$name$;\n");
  } else {
    p.Print(variables_,
            "$i$return typeof this._raw[FIELD][$index$] == 'undefined' ? "
            "$default$ : this._raw[FIELD][$index$];\n");
  }
}

void FieldGenerator::genSet(google::protobuf::io::Printer& p) {
  if (is_message_) {
    p.Print(
        variables_,
        "$i$var msg = "
        "this._maybeConvertToMessage($message_scope$$message_type$, value);\n"
        "$i$this._$name$ = msg;\n"
        "$i$this._raw[FIELD][$index$] = msg._raw;\n");
  } else {
    // We reject undefined and treat it as default value.
    // TODO: Emit error if the argument is not of correct type.
    p.Print(variables_,
            "$i$if (typeof value == 'undefined') {\n"
            "$i$  this._raw[FIELD][$index$] = $default$;\n"
            "$i$} else {\n"
            "$i$  this._raw[FIELD][$index$] = value;\n"
            "$i$}\n");
  }
}

void FieldGenerator::genClear(google::protobuf::io::Printer& p) {
  if (is_message_) {
    p.Print(variables_,
            "$i$this._raw[FIELD][$index$] = undefined;\n"
            "$i$this._$name$ = undefined;\n");
  } else {
    p.Print(variables_, "$i$this._raw[FIELD][$index$] = $default$;\n");
  }
}

void FieldGenerator::generateOptionalProperty(
    google::protobuf::io::Printer& p) {
  p.Print(variables_, "$i$$type$.prototype.get$capital_name$ = function() {\n");

  if (oneof_) {
    p.Print(variables_,
            "$i$  if (this.$oneof_camel$Case != "
            "type.$oneof_capital$Case.$all_capital_name$) {\n"
            "$i$    return undefined;\n"
            "$i$  } else {\n");
    indentUp();
    indentUp();
    genGet(p);
    indentDown();
    indentDown();
    // TODO: Currently, if undefined is passed to oneof field, the entire oneof
    // is cleared. It may better to ignore undefined value if the4 field is not
    // the active value at the time.
    p.Print(variables_,
            "    }\n"
            "  };\n"
            "$i$$type$.prototype.set$capital_name$ = function(value) {\n"
            "$i$  this.clear$oneof_capital$();\n"
            "$i$  this._oneofs[$oneof_index$] = "
            "type.$oneof_capital$Case.$all_capital_name$;\n");
  } else {
    indentUp();
    genGet(p);
    indentDown();
    p.Print(variables_,
            "$i$}\n"
            "$i$$type$.prototype.set$capital_name$ = function(value) {\n");
  }

  indentUp();
  genSet(p);
  indentDown();

  p.Print(variables_,
          "$i$};\n"
          "$i$$type$.prototype.clear$capital_name$ = function() {\n");
  indentUp();
  genClear(p);
  indentDown();
  if (oneof_) {
    p.Print(variables_,
            "$i$  if (this.$oneof_camel$Case == "
            "type.$oneof_capital$Case.$all_capital_name$) {\n"
            "$i$    this._oneofs[$oneof_index$] = "
            "type.$oneof_capital$Case.$oneof_all_capital$_NOT_SET;\n"
            "$i$  }\n");
  }
  p.Print(variables_,
          "$i$};\n"
          "$i$Object.defineProperty(type.prototype, '$name$', {\n"
          "$i$  get: $type$.prototype.get$capital_name$,\n"
          "$i$  set: $type$.prototype.set$capital_name$,\n"
          "$i$});\n\n");
}

void FieldGenerator::generateProperty(google::protobuf::io::Printer& p) {
  if (t_->is_repeated()) {
    generateRepeatedProperty(p);
  } else {
    generateOptionalProperty(p);
  }
}
}
}
