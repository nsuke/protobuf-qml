#include "protobuf/qml/field_generator.h"
#include "protobuf/qml/enum_generator.h"
#include "protobuf/qml/util.h"
#include "protobuf/qml/compiler/util.h"
#include <QDebug>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

FieldGenerator::FieldGenerator(const FieldDescriptor* t)
    : t_(t), camel_name_(t_ ? camelize(t_->name()) : "") {
  capital_name_ = capitalizeFirstLetter(camel_name_);
  if (t_) {
    variables_ = {
        {"type", "type"},
        {"default", defaultValue()},
        {"name", camel_name_},
        {"capital_name", capital_name_},
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
    if (auto oneof = t_->containing_oneof()) {
      auto oneof_camel = camelize(oneof->name());
      auto oneof_capital = capitalizeFirstLetter(oneof_camel);
      variables_.insert(std::make_pair("oneof_camel", std::move(oneof_camel)));
      variables_.insert(
          std::make_pair("oneof_capital", std::move(oneof_capital)));
      variables_.insert(
          std::make_pair("oneof_all_capital", capitalizeAll(oneof->name())));
      variables_.insert(
          std::make_pair("oneof_index", std::to_string(oneof->index())));
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
      return "'" + t_->default_value_string() + "'";
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "{}";
    default:
      qDebug() << t_->cpp_type();
      Q_ASSERT(false);
  }
}

void FieldGenerator::generateInit(io::Printer& p) {
  if (t_->is_repeated()) {
    if (t_->message_type()) {
      p.Print("    this._$name$ = new Array();\n", "name", camel_name_);
    }
    p.Print(
        variables_,
        "    this._raw[FIELD][$index$] = new Array();\n"
        "    if (values && values.$name$ && values.$name$ instanceof Array) {\n"
        "      this.$name$(values.$name$);\n"
        "    }\n");
  } else {
    p.Print(variables_, "    this.$name$($default$);\n");
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
        "        this.$name$()._mergeFromRawArray($arg$[FIELD][$index$]);\n");
  } else {
    p.Print("        this.$name$($arg$[FIELD][$index$]);\n", "name",
            camel_name_, "arg", arg, "index", std::to_string(t_->index()));
  }
}

void FieldGenerator::generateRepeatedMessageProperty(
    google::protobuf::io::Printer& p) {
  p.Print(
      variables_,
      "  $type$.prototype.$name$ = function(indexOrValues, value) {\n"
      "    if (typeof indexOrValues == 'undefined') {\n"
      "      return;\n"
      "    }\n"
      "    if (indexOrValues instanceof Array) {\n"
      "      this._$name$.length = indexOrValues.length;\n"
      "      this._raw[FIELD][$index$].length = indexOrValues.length;\n"
      "      for (var i in indexOrValues) {\n"
      "        var msg = new $message_scope$$message_type$(indexOrValues[i]);\n"
      "        this._$name$[i] = msg;\n"
      "        this._raw[FIELD][$index$][i] = msg._raw;\n"
      "      }\n"
      "      return;\n"
      "    }\n"
      "    if (typeof indexOrValues != 'number') {\n"
      "      throw new TypeError('Index should be a number.');\n"
      "    }\n"
      "    if (typeof value == 'undefined') {\n"
      "      return this._$name$[indexOrValues];\n"
      "    } else {\n"
      "      var msg = new $message_scope$$message_type$(value);\n"
      "      this._$name$[indexOrValues] = msg;\n"
      "      this._raw[FIELD][$index$][indexOrValues] = msg._raw;\n"
      "    }\n"
      "  };\n"
      "  $type$.prototype.$name$Count = function() {\n"
      "    console.assert(this._$name$.length == "
      "this._raw[FIELD][$index$].length);\n"
      "    return this._$name$.length;\n"
      "  };\n"
      "  $type$.prototype.add$capital_name$ = function(value) {\n"
      "    if (typeof value == 'undefined') {\n"
      "      throw new TypeError('Cannot add undefined.');\n"
      "    }\n"
      "    var msg = new $message_scope$$message_type$(value);\n"
      "    this._$name$.push(msg);\n"
      "    this._raw[FIELD][$index$].push(msg._raw);\n"
      "    console.assert(this._$name$.length == "
      "this._raw[FIELD][$index$].length);\n"
      "  };\n"
      "  $type$.prototype.remove$capital_name$ = function(index) {\n"
      "    if (typeof index != 'number') {\n"
      "      throw new TypeError('Index should be a number.');\n"
      "    }\n"
      "    this._raw[FIELD][$index$].splice(index, 1);\n"
      "    this._$name$.splice(index, 1);\n"
      "    console.assert(this._$name$.length == "
      "this._raw[FIELD][$index$].length);\n"
      "  };\n"
      "  $type$.prototype.clear$capital_name$ = function() {\n"
      "    this._raw[FIELD][$index$].length = 0;\n"
      "    this._$name$.length = 0;\n"
      "    console.assert(this._$name$.length == "
      "this._raw[FIELD][$index$].length);\n"
      "  };\n");
}

void FieldGenerator::generateRepeatedProperty(
    google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "  $type$.prototype.$name$ = function(indexOrValues, value) {\n"
          "    if (typeof indexOrValues == 'undefined') {\n"
          "      return;\n"
          "    }\n"
          "    if (indexOrValues instanceof Array) {\n"
          "      this._raw[FIELD][$index$].length = indexOrValues.length;\n"
          "      for (var i in indexOrValues) {\n"
          "        this._raw[FIELD][$index$][i] = indexOrValues[i];\n"
          "      }\n"
          "      return;\n"
          "    }\n"
          "    if (typeof indexOrValues != 'number') {\n"
          "      throw new TypeError('Index should be a number.');\n"
          "    }\n"
          "    if (typeof value == 'undefined') {\n"
          "      return this._raw[FIELD][$index$][indexOrValues];\n"
          "    } else {\n"
          "      this._raw[FIELD][$index$][indexOrValues] = value;\n"
          "    }\n"
          "  };\n"
          "  $type$.prototype.$name$Count = function() {\n"
          "    return this._raw[FIELD][$index$].length;\n"
          "  };\n"
          "  $type$.prototype.add$capital_name$ = function(value) {\n"
          "    if (typeof value == 'undefined') {\n"
          "      throw new TypeError('Cannot add undefined.');\n"
          "    }\n"
          "    this._raw[FIELD][$index$].push(value);\n"
          "  };\n"
          "  $type$.prototype.remove$capital_name$ = function(index) {\n"
          "    if (typeof index != 'number') {\n"
          "      throw new TypeError('Index should be a number.');\n"
          "    }\n"
          "    this._raw[FIELD][$index$].splice(index, 1);\n"
          "  };\n"
          "  $type$.prototype.clear$capital_name$ = function() {\n"
          "    this._raw[FIELD][$index$].length = 0;\n"
          "  };\n");
}

void FieldGenerator::generateOptionalMessageProperty(
    google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "  $type$.prototype.$name$ = function(value) {\n"
          "    if (typeof value == 'undefined') {\n"
          "      return this._$name$;\n"
          "    } else {\n"
          "      var msg = new $message_scope$$message_type$(value);\n"
          "      this._$name$ = msg;\n"
          "      this._raw[FIELD][$index$] = msg._raw;\n"
          "    }\n"
          "  };\n"
          "  $type$.prototype.clear$capital_name$ = function(value) {\n"
          "    this._raw[FIELD][$index$] = undefined;\n"
          "    this._$name$ = undefined;\n"
          "  };\n");
}

void FieldGenerator::generateOptionalProperty(
    google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "  $type$.prototype.$name$ = function(value) {\n"
          "    if (typeof value == 'undefined') {\n"
          "      return this._raw[FIELD][$index$];\n"
          "    } else {\n");
  auto oneof = t_->containing_oneof();
  if (oneof) {
    p.Print(variables_,
            "      this.clear$oneof_capital$();\n"
            "      this._raw[ONEOF][$oneof_index$] = "
            "type.$oneof_capital$Case.$capital_name$;\n");
  }
  p.Print(variables_,
          "      this._raw[FIELD][$index$] = value;\n"
          "    }\n"
          "  };\n"
          "  $type$.prototype.clear$capital_name$ = function(value) {\n");
  if (oneof) {
    p.Print(variables_,
            "    if (this.$oneof_camel$Case() == "
            "type.$oneof_capital$Case.$capital_name$) {\n"
            "      this._raw[ONEOF][$oneof_index$] = type.$oneof_capital$Case.$oneof_all_capital$_NOT_SET;\n"
            "    }\n");
  }
  p.Print(variables_,
          "    this._raw[FIELD][$index$] = $default$;\n"
          "  };\n");
}

void FieldGenerator::generateProperty(google::protobuf::io::Printer& p) {
  if (t_->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE && t_->is_repeated()) {
    generateRepeatedMessageProperty(p);
  } else if (t_->is_repeated()) {
    generateRepeatedProperty(p);
  } else if (t_->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    generateOptionalMessageProperty(p);
  } else {
    generateOptionalProperty(p);
  }
}
}
}
