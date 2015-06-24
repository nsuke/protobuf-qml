#include "protobuf/qml/oneof_generator.h"
#include "protobuf/qml/util.h"
#include "protobuf/qml/compiler_common.h"
#include "protobuf/qml/field_generator.h"

namespace protobuf {
namespace qml {

const char* containing_type = "type";

using namespace google::protobuf;

OneofGenerator::OneofGenerator(const google::protobuf::OneofDescriptor* t)
    : t_(t),
      camel_name_(t ? camelize(t->name()) : ""),
      all_capital_name_(t ? capitalizeAll(t->name()) : "") {
  capital_name_ = capitalizeFirstLetter(camel_name_);
  variables_ = {
      {"type", containing_type},
      {"name", camel_name_},
      {"capital_name", capital_name_},
      {"all_capital_name", all_capital_name_},
      {"index", std::to_string(t_->index())},
  };
}

void OneofGenerator::generateMerge(google::protobuf::io::Printer& p,
                                   const std::string& arg) {
  auto v = variables_;
  v.insert(std::make_pair("arg", arg));
  p.Print(v, "    switch ($arg$[ONEOF][$index$]) {\n");

  for (int i = 0; i < t_->field_count(); ++i) {
    p.Print("      case $oneof_number$: {\n", "oneof_number",
            std::to_string(t_->field(i)->number()));
    FieldGenerator fg(t_->field(i));
    fg.generateMerge(p, arg);
    p.Print(
        "        break;\n"
        "      }\n");
  }

  p.Print(variables_,
          "      default:\n"
          "        this.clear$capital_name$();\n"
          "        break;\n"
          "    }\n");
}

void OneofGenerator::generateInit(google::protobuf::io::Printer& p) {
  p.Print(variables_, "    this.clear$capital_name$();\n");
}

void OneofGenerator::generate(io::Printer& p) {
  generateCaseEnum(p);
  generateCase(p);
  generateClear(p);
}

void OneofGenerator::generateCaseEnum(io::Printer& p) {
  p.Print(variables_, "  $type$.$capital_name$Case = {\n");
  for (int i = 0; i < t_->field_count(); ++i) {
    auto f = t_->field(i);
    p.Print("    get $all_capital_name$() { return $number$;},\n",
            "all_capital_name", capitalizeAll(f->name()), "number",
            std::to_string(f->number()));
  }
  p.Print(variables_,
          "    get $all_capital_name$_NOT_SET() { return 0; },\n"
          "  };\n"
          "  Object.freeze($type$.$capital_name$Case);\n\n");
}

void OneofGenerator::generateCase(io::Printer& p) {
  p.Print(variables_,
          "  $type$.prototype.$name$Case = function() {\n"
          "    return this._raw[ONEOF][$index$];\n"
          "  };\n");
}

void OneofGenerator::generateClear(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "  $type$.prototype.clear$capital_name$ = function() {\n"
          "    switch (this.$name$Case()) {\n");
  for (int i = 0; i < t_->field_count(); ++i) {
    auto f = t_->field(i);
    switch (f->cpp_type()) {
      case FieldDescriptor::CPPTYPE_STRING:
      case FieldDescriptor::CPPTYPE_MESSAGE:
        p.Print(
            "      case $type$.$capital_name$Case.$all_capital_name$: {\n"
            "        this.clear$name$();\n"
            "        break;\n"
            "      }\n",
            "name", capitalize(f->name()), "all_capital_name",
            capitalizeAll(f->name()), "capital_name", capital_name_, "type",
            containing_type);
        break;
      case FieldDescriptor::CPPTYPE_INT32:
      case FieldDescriptor::CPPTYPE_INT64:
      case FieldDescriptor::CPPTYPE_UINT32:
      case FieldDescriptor::CPPTYPE_UINT64:
      case FieldDescriptor::CPPTYPE_DOUBLE:
      case FieldDescriptor::CPPTYPE_FLOAT:
      case FieldDescriptor::CPPTYPE_BOOL:
      case FieldDescriptor::CPPTYPE_ENUM:
        break;
      default:
        PBQML_ASSERT_X(false, "generateClear", "Unknown cpp type");
        break;
    }
  }
  p.Print(variables_,
          // "      case $capital_name$Case.$all_capital_name$_NOT_SET: {\n"
          // "        break;\n"
          // "      }\n"
          "    }\n"
          "    this._raw[ONEOF][$index$] = "
          "$type$.$capital_name$Case.$all_capital_name$_NOT_SET;\n"
          "  };\n\n");
}
}
}
