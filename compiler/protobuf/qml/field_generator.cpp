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
      p.Print(variables_, "$i$this._fields[$index$] = new $typed_array$();\n");
    } else if (t_->message_type()) {
      p.Print(variables_, "$i$this._fields[$index$] = new Array();\n");
    } else {
      p.Print(variables_, "$i$this._fields[$index$] = new Array();\n");
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
            "$i$field = $arg$[0][$index$];\n"
            "$i$if (field instanceof Array) {\n"
            "$i$  for (var i = 0; i < field.length; ++i) {\n"
            "$i$    if (typeof this._$name$[i] == 'undefined') {\n"
            "$i$      var msg = new $message_scope$$message_type$();\n"
            "$i$      this._$name$[i] = msg;\n"
            "$i$      this._fields[$index$][i] = msg._raw;\n"
            "$i$    }\n"
            "$i$    this._$name$[i]._mergeFromRawArray(field[i]);\n"
            "$i$  }\n"
            "$i$}\n");
  } else if (t_->is_repeated() && is_typed_array_) {
    p.Print(v,
            "$i$field = $arg$[0][$index$];\n"
            "$i$if (field instanceof Array || field instanceof "
            "$typed_array$ || field instanceof ArrayBuffer) {\n"
            "$i$  this.set$capital_name$(field);\n"
            "$i$}\n");
  } else if (t_->is_repeated()) {
    p.Print(v,
            "$i$if ($arg$[0][$index$] instanceof Array) {\n"
            "$i$  this.$name$($arg$[0][$index$]);\n"
            "$i$}\n");
  } else if (t_->message_type()) {
    p.Print(v,
            "$i$if (typeof this.$name$ == 'undefined') {\n"
            "$i$  this.set$capital_name$({});\n"
            "$i$}\n"
            "$i$this._$name$._mergeFromRawArray($arg$[0][$index$]);\n");
  } else {
    p.Print(v, "$i$this.set$capital_name$($arg$[0][$index$]);\n");
  }
}

void FieldGenerator::messageAssertLength(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "$i$console.assert(this._$name$.length == "
          "this._fields[$index$].length);\n");
}

void FieldGenerator::countMethod(google::protobuf::io::Printer& p) {
  p.Print(variables_, "$i$var $name$Count = function() {\n");

  if (is_message_) {
    if (is_message_) messageAssertLength(p);
    p.Print(variables_, "$i$  return this._$name$.length;\n");
  } else {
    p.Print(variables_, "$i$  return this._fields[$index$].length;\n");
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
          "$i$$type$.prototype.get$capital_name$Length = $name$Count;\n");
}

void FieldGenerator::addMethod(google::protobuf::io::Printer& p) {
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
            "$i$  this._fields[$index$].push(msg._raw);\n");
    messageAssertLength(p);
  } else if (is_typed_array_) {
    p.Print(variables_,
            "$i$  var newArray = this._ensureLength(this._fields[$index$]);\n"
            "$i$  if (newArray) {\n"
            "$i$    this._fields[$index$] = newArray;\n"
            "$i$  }\n"
            "$i$  this._fields[$index$][this._fields[$index$].length - 1] "
            "= value;\n");
  } else {
    p.Print(variables_, "$i$  this._fields[$index$].push(value);\n");
  }

  p.Print(variables_, "$i$};\n");
}

void FieldGenerator::reserveMethod(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "$i$$type$.prototype.reserve$capital_name$ = function(len) {\n");

  if (is_typed_array_) {
    p.Print(variables_,
            "$i$  var blen = len * $typed_array$.BYTES_PER_ELEMENT;\n"
            "$i$  if (this._fields[$index$].buffer.byteLength < blen) {\n"
            "$i$    var buf = new ArrayBuffer(blen);\n"
            "$i$    var newArray = new $typed_array$(buf, 0, "
            "this._fields[$index$].length);\n"
            "$i$    newArray.set(this._fields[$index$]);\n"
            "$i$    this._fields[$index$] = newArray;\n"
            "$i$  }\n");
  } else {
    p.Print(variables_, "$i$  // NOOP");
  }
  p.Print(variables_, "$i$};\n");
}

void FieldGenerator::resizeMethod(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "$i$$type$.prototype.resize$capital_name$ = function(len) {\n");

  if (is_typed_array_) {
    p.Print(
        variables_,
        "$i$  var newArray = this._ensureLength(this._fields[$index$], len);\n"
        "$i$  if (newArray) {\n"
        "$i$    newArray.set(this._fields[$index$]);\n"
        "$i$    this._fields[$index$] = newArray;\n"
        "$i$  }\n"
        "$i$  console.assert(this._fields[$index$].length >= len);\n");
  } else if (!is_message_) {
    p.Print(variables_, "$i$  this._fields[$index$].length = values.length;\n");
  } else {
    p.Print(variables_, "$i$  // NOOP");
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
            // Omit type checks for performance
            // "$i$  if (values instanceof $typed_array$ || values "
            // "instanceof Array "
            // "|| values instanceof ArrayBuffer) {\n"
            "$i$    if (!values.BYTES_PER_ELEMENT) {\n"
            "$i$      values = new $typed_array$(values);;\n"
            "$i$    }\n"
            "$i$    var newArray = this._ensureLength(this._fields[$index$], "
            "values.length);\n"
            "$i$    if (newArray) {\n"
            "$i$      this._fields[$index$] = newArray;\n"
            "$i$    }\n"
            "$i$    this._fields[$index$].set(values);\n"
            // "$i$  } else {\n"
            // "$i$    throw new TypeError();\n"
            // "$i$  }\n"
            );
  } else {
    p.Print(variables_,
            "$i$  if (!(values instanceof Array)) {\n"
            "$i$    throw new TypeError();\n"
            "$i$  }\n");
    if (is_message_) {
      p.Print(variables_,
              "$i$  this._fields[$index$].length = values.length;\n"
              "$i$   this._$name$.length = values.length;\n"
              "$i$   for (var i = 0; i < values.length; ++i) {\n"
              "$i$     var msg = new "
              "$message_scope$$message_type$(values[i]);\n"
              "$i$     this._$name$[i] = msg;\n"
              "$i$     this._fields[$index$][i] = msg._raw;\n"
              "$i$   }\n");
    } else {
      p.Print(variables_, "$i$    this._fields[$index$] = values.slice();\n");
    }
  }
  p.Print(variables_,
          "$i$};\n"
          "$i$// Single value setter\n"
          "$i$$type$.prototype.set$capital_name$At = function(index, value) "
          "{\n"
          // Type checks are expensive for V4 engine.
          //"$i$  if (typeof index != 'number') {\n"
          //"$i$    throw new TypeError('Index should be a number: ' + typeof "
          //"index);\n"
          //"$i$  }\n"
          //"$i$  if(this._fields[$index$].length <= index) {\n"
          //"$i$    throw new RangeError();\n"
          //"$i$  }\n");
          );
  if (is_message_) {
    p.Print(variables_,
            "$i$  var msg = "
            "this._maybeConvertToMessage($message_scope$$message_type$, "
            "value);\n"
            "$i$  this._$name$[index] = msg;\n"
            "$i$  this._fields[$index$][index] = msg._raw;\n");
  } else if (is_typed_array_) {
    p.Print(variables_, "$i$  this._fields[$index$][index] = value;\n");
  } else {
    p.Print(variables_, "$i$  this._fields[$index$][index] = value;\n");
  }
  p.Print(variables_,
          "$i$};\n"
          "$i$// Getter\n"
          "$i$$type$.prototype.get$capital_name$At = function(index) {\n"
          "$i$  if (typeof index != 'number') {\n"
          "$i$    throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "$i$  }\n"
          "$i$  if(this._fields[$index$].length < index) {\n"
          "$i$    throw new RangeError();\n"
          "$i$  }\n");
  if (is_message_) {
    p.Print(variables_, "$i$  return this._$name$[index];\n");
  } else {
    p.Print(variables_, "$i$  return this._fields[$index$][index];\n");
  }
  p.Print(variables_,
          "$i$};\n"
          "$i$$type$.prototype.get$capital_name$AsArray = function() {\n");
  if (is_message_) {
    p.Print(variables_, "$i$  return this._$name$.slice();\n");
  } else if (is_typed_array_) {
    p.Print(variables_,
            "$i$  var array = [];\n"
            "$i$  for (var i = 0; i < this._fields[$index$].length; ++i) {\n"
            "$i$    array.push(this._fields[$index$][i]);\n"
            "$i$  }\n"
            "$i$  return array;\n");
  } else {
    p.Print(variables_, "$i$  return this._fields[$index$].slice();\n");
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
          "$i$};\n");

  countMethod(p);
  addMethod(p);
  if (is_typed_array_) {
    reserveMethod(p);
  }
  if (!is_message_) {
    resizeMethod(p);
  }

  p.Print(variables_,
          "$i$$type$.prototype.remove$capital_name$ = function(index) {\n"
          "$i$  if (typeof index != 'number') {\n"
          "$i$    throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "$i$  }\n"
          "$i$  this._fields[$index$].splice(index, 1);\n");

  if (is_message_) {
    p.Print(variables_, "$i$  this._$name$.splice(index, 1);\n");
    messageAssertLength(p);
  }

  p.Print(variables_,
          "$i$};\n"
          "$i$$type$.prototype.clear$capital_name$ = function() {\n");

  if (is_typed_array_) {
    p.Print(variables_,
            "$i$  this._fields[$index$] = new "
            "$typed_array$(this._fields[$index$].buffer, 0, 0);\n");
  } else {
    p.Print(variables_, "$i$  this._fields[$index$].length = 0;\n");

    if (is_message_) {
      p.Print(variables_, "$i$  this._$name$.length = 0;\n");
      messageAssertLength(p);
    }
  }

  p.Print(variables_, "$i$};\n");
}

void FieldGenerator::genGet(google::protobuf::io::Printer& p) {
  if (is_message_) {
    p.Print(variables_, "$i$return this._$name$;\n");
  } else {
    p.Print(variables_,
            "$i$return typeof this._fields[$index$] == 'undefined' ? "
            "$default$ : this._fields[$index$];\n");
  }
}

void FieldGenerator::genSet(google::protobuf::io::Printer& p) {
  if (is_message_) {
    p.Print(
        variables_,
        "$i$var msg = "
        "this._maybeConvertToMessage($message_scope$$message_type$, value);\n"
        "$i$this._$name$ = msg;\n"
        "$i$this._fields[$index$] = msg._raw;\n");
  } else {
    // TODO: Emit error if the argument is not of correct type.
    p.Print(variables_,
            // We check for undefined in getters
            //"$i$if (typeof value == 'undefined') {\n"
            //"$i$  this._fields[$index$] = $default$;\n"
            //"$i$} else {\n"
            //"$i$  this._fields[$index$] = value;\n"
            //"$i$}\n");
            "$i$  this._fields[$index$] = value;\n");
  }
}

void FieldGenerator::genClear(google::protobuf::io::Printer& p) {
  if (is_message_) {
    p.Print(variables_,
            "$i$this._fields[$index$] = undefined;\n"
            "$i$this._$name$ = undefined;\n");
  } else {
    p.Print(variables_, "$i$this._fields[$index$] = $default$;\n");
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
