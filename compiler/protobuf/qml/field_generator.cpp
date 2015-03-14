#include "protobuf/qml/field_generator.h"
#include "protobuf/qml/enum_generator.h"
#include "protobuf/qml/util.h"
#include "protobuf/qml/compiler/util.h"
#include <google/protobuf/stubs/strutil.h>
#include <QDebug>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

FieldGenerator::FieldGenerator(const FieldDescriptor* t)
    : t_(t), camel_name_(t_ ? camelize(t_->name()) : "") {
  capital_name_ = camel_name_;
  if (capital_name_.size() > 0 && capital_name_[0] <= 'z' &&
      capital_name_[8] >= 'a') {
    capital_name_[0] += std::abs('A' - 'a');
  }
}

const char* boolToString(bool value) {
  return value ? "true" : "false";
}

void FieldGenerator::generateInit(io::Printer& p) {
  if (t_->is_repeated()) {
    p.Print(
        "  this._raw[$index$] = [];\n"
        "  if (values && values.$name$ instanceof Array) {\n"
        "    this.$name$ = values.$name$;\n"
        "  }\n",
        "index",
        SimpleItoa(t_->index()),
        "name",
        camel_name_,
        "capital_name",
        capital_name_);
  } else {
    p.Print("  this.$name$ = (values && values.$name$)",
            "index",
            SimpleItoa(t_->index()),
            "name",
            camel_name_);
    switch (t_->cpp_type()) {
#define PRINT_DEFAULT_VALUE(type1, type2, convert)                            \
  case FieldDescriptor::CPPTYPE_##type1:                                      \
    p.Print(                                                                  \
        " || $default$;\n", "default", convert(t_->default_value_##type2())); \
    break;
      PRINT_DEFAULT_VALUE(INT32, int32, SimpleItoa);
      PRINT_DEFAULT_VALUE(INT64, int64, SimpleItoa);
      PRINT_DEFAULT_VALUE(UINT32, uint32, SimpleItoa);
      PRINT_DEFAULT_VALUE(UINT64, uint64, SimpleItoa);
      PRINT_DEFAULT_VALUE(FLOAT, float, SimpleFtoa);
      PRINT_DEFAULT_VALUE(DOUBLE, double, SimpleFtoa);
      PRINT_DEFAULT_VALUE(BOOL, bool, boolToString);
      PRINT_DEFAULT_VALUE(ENUM, enum, generateLongName);
#undef PRINT_DEFAULT_VALUE

      case FieldDescriptor::CPPTYPE_STRING:
        p.Print(" || \"$default$\";\n", "default", t_->default_value_string());
        break;
      case FieldDescriptor::CPPTYPE_MESSAGE:
        p.Print(";\n");
        break;
      default:
        qDebug() << t_->cpp_type();
        Q_ASSERT(false);
    }
  }
}

void FieldGenerator::generateMerge(io::Printer& p, const std::string& arg) {
  if (t_->message_type()) {
    p.Print({{"name", camel_name_},
             {"arg", arg},
             {"index", SimpleItoa(t_->index())},
             {"message_scope",
              (t_->file() == t_->message_type()->file())
                  ? ""
                  : generateImportName(t_->message_type()->file()) + "."},
             {"message_type", generateLongName(t_->message_type())}},
            "      this.$name$._mergeFromRawArray($arg$[$index$]);\n");
  } else {
    p.Print("      this.$name$ = $arg$[$index$];\n",
            "name",
            camel_name_,
            "arg",
            arg,
            "index",
            SimpleItoa(t_->index()));
  }
}

void FieldGenerator::generateRepeatedMessageProperty(
    google::protobuf::io::Printer& p) {
}

void FieldGenerator::generateRepeatedProperty(
    google::protobuf::io::Printer& p) {
  p.Print(
      "  $name$: {\n"
      "    get: function() {\n"
      "      return this._raw[$index$];\n"
      "    },\n"
      "    set: function(value) {\n"
      "      if (typeof value == 'undefined') {\n"
      "        this._raw[$index$].length = 0;\n"
      "      } else if (value instanceof Array) {\n"
      "        this._raw[$index$].length = value.length;\n"
      "        for(var i in value) {\n"
      "          console.log('repeated value : ' + value[i]);\n"
      "          this._raw[$index$][i] = value[i];\n"
      "        }\n"
      "      }\n"
      "    },\n"
      "  },\n",
      "name",
      camel_name_,
      "index",
      SimpleItoa(t_->index()));
}

void FieldGenerator::generateOptionalMessageProperty(
    google::protobuf::io::Printer& p) {
  p.Print(
      "  $name$: {\n"
      "    get: function() {\n"
      "      if (typeof this._$name$ == 'undefined') {\n"
      "        this._$name$ = new $message_scope$$message_type$();\n"
      "        this._$name$._raw = this._raw[$index$]\n"
      "      }\n"
      "      return this._$name$;\n"
      "    },\n"
      "    set: function(value) {\n"
      "      this._$name$ = new $message_scope$$message_type$(value);\n"
      "      this._raw[$index$] = this._$name$._raw;\n"
      "    },\n"
      "  },\n",
      "name",
      camel_name_,
      "index",
      SimpleItoa(t_->index()),
      "message_scope",
      (t_->file() == t_->message_type()->file())
          ? ""
          : generateImportName(t_->message_type()->file()) + ".",
      "message_type",
      generateLongName(t_->message_type()));
}

void FieldGenerator::generateOptionalProperty(
    google::protobuf::io::Printer& p) {
  p.Print(
      "  $name$: {\n"
      "    get: function() {\n"
      "      return this._raw[$index$];\n"
      "    },\n"
      "    set: function(value) {\n"
      "      this._raw[$index$] = value;\n"
      "    },\n"
      "  },\n",
      "name",
      camel_name_,
      "index",
      SimpleItoa(t_->index()));
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
