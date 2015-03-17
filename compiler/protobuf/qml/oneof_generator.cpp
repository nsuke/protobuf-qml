#include "protobuf/qml/oneof_generator.h"
#include "protobuf/qml/util.h"
#include <qglobal.h>

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

void OneofGenerator::generate(io::Printer& p) {
  generateCaseEnum(p);
  generateCase(p);
  generateClear(p);
}

void OneofGenerator::generateCaseEnum(io::Printer& p) {
  p.Print(variables_, "  $type$.$capital_name$Case = {\n");
  for (int i = 0; i < t_->field_count(); ++i) {
    auto f = t_->field(i);
    p.Print("    get $capital_name$() { return $number$;},\n", "capital_name",
            capitalize(f->name()), "number", std::to_string(f->number()));
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
        p.Print("      case $type$.$capital_name$Case.$name$: {\n", "name",
                capitalize(f->name()), "capital_name", capital_name_, "type", containing_type);
        p.Print("        this.clear$name$();\n", "name", capitalize(f->name()));
        p.Print(
            "        break;\n"
            "      }\n");
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
        Q_ASSERT(false);
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
