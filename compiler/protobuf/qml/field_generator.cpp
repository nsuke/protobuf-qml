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
      is_typed_array_(t_ ? t_->cpp_type() == FieldDescriptor::CPPTYPE_INT32
                         : false),
      oneof_(t_ ? t_->containing_oneof() : nullptr),
      camel_name_(t_ ? camelize(t_->name()) : "") {
  capital_name_ = capitalizeFirstLetter(camel_name_);
  if (t_) {
    variables_ = {
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
      p.Print("    this._$name$ = new Array();\n", "name", camel_name_);
    }
    if (is_typed_array_) {
      p.Print(variables_,
              "    var buffer = new ArrayBuffer(32);\n"
              "    this._$name$ = new Int32Array(buffer, 0, 0);\n"
              "    this._raw[FIELD][$index$] = this._$name$;\n");
    } else if (t_->message_type()) {
      p.Print(variables_, "    this._raw[FIELD][$index$] = new Array();\n");
    } else {
      p.Print(variables_, "    this._raw[FIELD][$index$] = new Array();\n");
    }
    p.Print(
        variables_,
        "    if (values && values.$name$ && values.$name$ instanceof Array) {\n"
        "      this.$name$(values.$name$);\n"
        "    }\n");
  }
}

void FieldGenerator::generateMerge(io::Printer& p, const std::string& arg) {
  if (t_->is_repeated() && t_->message_type()) {
    auto v = variables_;
    v.insert(std::make_pair("arg", arg));
    p.Print(v,
            "        if ($arg$[FIELD][$index$] && $arg$[FIELD][$index$] "
            "instanceof Array) {\n"
            "          for (var i in $arg$[FIELD][$index$]) {\n"
            "            if (typeof this._$name$[i] == 'undefined') {\n"
            "              var msg = new $message_scope$$message_type$();\n"
            "              this._$name$[i] = msg;\n"
            "              this._raw[FIELD][$index$][i] = msg._raw;\n"
            "            }\n"
            "            "
            "this._$name$[i]._mergeFromRawArray($arg$[FIELD][$index$][i]);\n"
            "          }\n"
            "        }\n");
  } else if (t_->is_repeated() && is_typed_array_) {
    p.Print(
        "        if ($arg$[FIELD][$index$] && ($arg$[FIELD][$index$] "
        "instanceof "
        "Array || $arg$[FIELD][$index$] instanceof Int32Array) || "
        "$arg$[FIELD][$index$] instanceof ArrayBuffer) {\n"
        "          this.$name$($arg$[FIELD][$index$]);\n"
        "        }\n",
        "name", camel_name_, "arg", arg, "index", std::to_string(t_->index()));
  } else if (t_->is_repeated()) {
    p.Print(
        "        if ($arg$[FIELD][$index$] && $arg$[FIELD][$index$] instanceof "
        "Array) {\n"
        "          this.$name$($arg$[FIELD][$index$]);\n"
        "        }\n",
        "name", camel_name_, "arg", arg, "index", std::to_string(t_->index()));
  } else if (t_->message_type()) {
    auto v = variables_;
    v.insert(std::make_pair("arg", arg));
    p.Print(
        v,
        "        if (typeof this.$name$ == 'undefined') {\n"
        "          this.$name$ = {};\n"
        "        }\n"
        "        this._$name$._mergeFromRawArray($arg$[FIELD][$index$]);\n");
  } else {
    p.Print("        this.set$capital_name$($arg$[FIELD][$index$]);\n",
            "capital_name", capital_name_, "arg", arg, "index",
            std::to_string(t_->index()));
  }
}

void FieldGenerator::messageAssertLength(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "    console.assert(this._$name$.length == "
          "this._raw[FIELD][$index$].length);\n");
}

void FieldGenerator::generateRepeatedProperty(
    google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "    // Replacement setter\n"
          "  $type$.prototype.set$capital_name$ = function(values) {\n");
  if (is_typed_array_) {
    p.Print(variables_,
            "    if (values instanceof Int32Array || values instanceof Array "
            "|| values instanceof ArrayBuffer) {\n"
            "      if (!(values instanceof Array) && !values.name) {\n"
            "        values = new Int32Array(values);;\n"
            "      }\n"
            "      var newArray = this._ensureLength(this._$name$, "
            "values.length);\n"
            "      if (newArray) {\n"
            "        this._$name$ = newArray;\n"
            "        this._raw[FIELD][$index$] = this._$name$;\n"
            "      }\n"
            "      this._$name$.set(values);\n"
            "    } else {\n"
            "      throw new TypeError();\n"
            "    }\n");
  } else {
    p.Print(variables_,
            "    if (!(values instanceof Array)) {\n"
            "      throw new TypeError();\n"
            "    }\n");
  }
  if (is_message_) {
    p.Print(variables_,
            "    this._raw[FIELD][$index$].length = values.length;\n"
            "     this._$name$.length = values.length;\n"
            "     for (var i in values) {\n"
            "       var msg = new "
            "$message_scope$$message_type$(values[i]);\n"
            "       this._$name$[i] = msg;\n"
            "       this._raw[FIELD][$index$][i] = msg._raw;\n"
            "     }\n");
  } else if (!is_typed_array_) {
    p.Print(variables_, "      this._raw[FIELD][$index$] = values.slice();\n");
  }
  p.Print(variables_,
          "  };\n"
          "  // Single value setter\n"
          "  $type$.prototype.set$capital_name$At = function(index, value) {\n"
          "    if (typeof index != 'number') {\n"
          "      throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "    }\n"
          "    if(this.$name$Size < index) {\n"
          "      throw new RangeError();\n"
          "    }\n");
  if (is_message_) {
    p.Print(variables_,
            "    var msg = new $message_scope$$message_type$(value);\n"
            "    this._$name$[index] = msg;\n"
            "    this._raw[FIELD][$index$][index] = msg._raw;\n");
  } else if (is_typed_array_) {
    p.Print(variables_, "    this._$name$[index] = value;\n");
  } else {
    p.Print(variables_, "    this._raw[FIELD][$index$][index] = value;\n");
  }
  p.Print(variables_,
          "  };\n"
          "  // Getter\n"
          "  $type$.prototype.get$capital_name$At = function(index) {\n"
          "    if (typeof index != 'number') {\n"
          "      throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "    }\n"
          "    if(this._raw[FIELD][$index$].length < index) {\n"
          "      throw new RangeError();\n"
          "    }\n");
  if (is_message_ || is_typed_array_) {
    p.Print(variables_, "    return this._$name$[index];\n");
  } else {
    p.Print(variables_, "    return this._raw[FIELD][$index$][index];\n");
  }
  p.Print(variables_,
          "  };\n"
          "  $type$.prototype.get$capital_name$AsArray = function() {\n");
  if (is_message_) {
    p.Print(variables_, "    return this._$name$.slice();\n");
  } else if (is_typed_array_) {
    p.Print(variables_,
            "    var array = [];\n"
            "    for (var i = 0; i < this._$name$.length; ++i) {\n"
            "      array.push(this._$name$[i]);\n"
            "    }\n"
            "    return array;\n");
  } else {
    p.Print(variables_, "    return this._raw[FIELD][$index$].slice();\n");
  }
  p.Print("  };\n");

  p.Print(variables_,
          "  $type$.prototype.$name$ = function(indexOrValues, value) {\n"
          "    if (typeof indexOrValues == 'undefined') {\n"
          "      throw new TypeError('Not enough arguments');\n"
          "    } else if (typeof indexOrValues == 'number') {\n"
          "      if (typeof value == 'undefined') {\n"
          "        return this.get$capital_name$At(indexOrValues);\n"
          "      } else {\n"
          "        this.set$capital_name$At(indexOrValues, value);\n"
          "      }\n"
          "    } else {\n"
          "      this.set$capital_name$(indexOrValues);\n"
          "    }\n"
          "  };\n"
          "  var $name$Count = function() {\n");

  if (is_typed_array_ || is_message_) {
    if (is_message_) messageAssertLength(p);
    p.Print(variables_, "    return this._$name$.length;\n");
  } else {
    p.Print(variables_, "    return this._raw[FIELD][$index$].length;\n");
  }

  p.Print(variables_,
          "  };\n"
          "  Object.defineProperties($type$.prototype, {\n"
          "    $name$Count: { get: $name$Count },\n"
          "    $name$Size: { get: $name$Count },\n"
          "    $name$Length: { get: $name$Count },\n"
          "  });\n"
          "  $type$.prototype.get$capital_name$Count = $name$Count;\n"
          "  $type$.prototype.get$capital_name$Size = $name$Count;\n"
          "  $type$.prototype.get$capital_name$Length = $name$Count;\n"
          "  $type$.prototype.add$capital_name$ = function(value) {\n"
          "    if (typeof value == 'undefined') {\n"
          "      throw new TypeError('Cannot add undefined.');\n"
          "    }\n");

  if (is_message_) {
    p.Print(variables_,
            "    var msg = new $message_scope$$message_type$(value);\n"
            "    this._$name$.push(msg);\n"
            "    this._raw[FIELD][$index$].push(msg._raw);\n");
    messageAssertLength(p);
  } else if (is_typed_array_) {
    p.Print(variables_,
            "    var newArray = this._ensureLength(this._$name$);\n"
            "    if (newArray) {\n"
            "      this._$name$ = newArray;\n"
            "      this._raw[FIELD][$index$] = this._$name$;\n"
            "    }\n"
            "    this._$name$[this._$name$.length - 1] = value;\n");
  } else {
    p.Print(variables_, "    this._raw[FIELD][$index$].push(value);\n");
  }

  p.Print(variables_,
          "  };\n"
          "  $type$.prototype.remove$capital_name$ = function(index) {\n"
          "    if (typeof index != 'number') {\n"
          "      throw new TypeError('Index should be a number: ' + typeof "
          "index);\n"
          "    }\n"
          "    this._raw[FIELD][$index$].splice(index, 1);\n");

  if (is_message_) {
    p.Print(variables_, "    this._$name$.splice(index, 1);\n");
    messageAssertLength(p);
  }

  p.Print(variables_,
          "  };\n"
          "  $type$.prototype.clear$capital_name$ = function() {\n"
          "    this._raw[FIELD][$index$].length = 0;\n");

  if (is_message_) {
    p.Print(variables_, "    this._$name$.length = 0;\n");
    messageAssertLength(p);
  }

  p.Print("  };\n");
}

void FieldGenerator::genGet(google::protobuf::io::Printer& p,
                            std::string indent) {
  auto v = variables_;
  v.insert(std::make_pair("indent", indent));
  if (is_message_) {
    p.Print(v, "$indent$return this._$name$;\n");
  } else {
    p.Print(v,
            "$indent$return typeof this._raw[FIELD][$index$] == 'undefined' ? "
            "$default$ : this._raw[FIELD][$index$];\n");
  }
}

void FieldGenerator::genSet(google::protobuf::io::Printer& p,
                            std::string indent) {
  auto v = variables_;
  v.insert(std::make_pair("indent", indent));
  if (is_message_) {
    p.Print(v,
            "$indent$var msg = new $message_scope$$message_type$(value);\n"
            "$indent$this._$name$ = msg;\n"
            "$indent$this._raw[FIELD][$index$] = msg._raw;\n");
  } else {
    // We reject undefined and treat it as default value.
    // TODO: Emit error if the argument is not of correct type.
    p.Print(v,
            "if (typeof value == 'undefined') {\n"
            "  $indent$this._raw[FIELD][$index$] = $default$;\n"
            "} else {\n"
            "  $indent$this._raw[FIELD][$index$] = value;\n"
            "}\n");
  }
}

void FieldGenerator::genClear(google::protobuf::io::Printer& p,
                              std::string indent) {
  auto v = variables_;
  v.insert(std::make_pair("indent", indent));
  if (is_message_) {
    p.Print(v,
            "$indent$this._raw[FIELD][$index$] = undefined;\n"
            "$indent$this._$name$ = undefined;\n");
  } else {
    p.Print(v, "$indent$this._raw[FIELD][$index$] = $default$;\n");
  }
}

void FieldGenerator::generateOptionalProperty(
    google::protobuf::io::Printer& p) {
  p.Print(variables_, "  $type$.prototype.get$capital_name$ = function() {\n");

  if (oneof_) {
    p.Print(variables_,
            "      if (this.$oneof_camel$Case != "
            "type.$oneof_capital$Case.$all_capital_name$) {\n"
            "        return undefined;\n"
            "      } else {\n");
    genGet(p, "        ");
    // TODO: Currently, if undefined is passed to oneof field, the entire oneof
    // is cleared. It may better to ignore undefined value if the4 field is not
    // the active value at the time.
    p.Print(variables_,
            "      }\n"
            "  };\n"
            "  $type$.prototype.set$capital_name$ = function(value) {\n"
            "      this.clear$oneof_capital$();\n"
            "      this._oneofs[$oneof_index$] = "
            "type.$oneof_capital$Case.$all_capital_name$;\n");
  } else {
    genGet(p, "    ");
    p.Print(variables_,
            "  }\n"
            "  $type$.prototype.set$capital_name$ = function(value) {\n");
  }
  genSet(p, "    ");
  p.Print(variables_,
          "  };\n"
          "  $type$.prototype.clear$capital_name$ = function() {\n");
  genClear(p, "    ");
  if (oneof_) {
    p.Print(variables_,
            "    if (this.$oneof_camel$Case == "
            "type.$oneof_capital$Case.$all_capital_name$) {\n"
            "      this._oneofs[$oneof_index$] = "
            "type.$oneof_capital$Case.$oneof_all_capital$_NOT_SET;\n"
            "    }\n");
  }
  p.Print(variables_,
          "  };\n"
          "  Object.defineProperty(type.prototype, '$name$', {\n"
          "    get: $type$.prototype.get$capital_name$,\n"
          "    set: $type$.prototype.set$capital_name$,\n"
          "  });\n\n");
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
