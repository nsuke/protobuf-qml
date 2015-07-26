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
      {"i", "  "},  // indent
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
  p.Print(v, "$i$switch (oneofs[$index$]) {\n");

  for (int i = 0; i < t_->field_count(); ++i) {
    v["oneof_number"] = std::to_string(t_->field(i)->number());
    p.Print(v, "$i$  case $oneof_number$: {\n");
    FieldGenerator fg(t_->field(i));
    auto prev = fg.indent();
    fg.indent(indent() + "    ");
    fg.generateMerge(p, arg);
    fg.indent(prev);
    p.Print(v,
            "$i$    break;\n"
            "$i$  }\n");
  }

  p.Print(v,
          "$i$  default:\n"
          "$i$    this.clear$capital_name$();\n"
          "$i$    break;\n"
          "$i$}\n");
}

void OneofGenerator::generateInit(google::protobuf::io::Printer& p) {
  p.Print(variables_, "$i$  this.clear$capital_name$();\n");
}

void OneofGenerator::generate(io::Printer& p) {
  generateCaseEnum(p);
  generateCase(p);
  generateClear(p);
}

void OneofGenerator::generateCaseEnum(io::Printer& p) {
  p.Print(variables_, "$i$$type$.$capital_name$Case = {\n");
  for (int i = 0; i < t_->field_count(); ++i) {
    auto f = t_->field(i);
    p.Print("$i$  get $all_capital_name$() { return $number$;},\n",
            "all_capital_name", capitalizeAll(f->name()), "number",
            std::to_string(f->number()), "i", variables_["i"]);
  }
  p.Print(variables_,
          "$i$  get $all_capital_name$_NOT_SET() { return 0; },\n"
          "$i$};\n"
          "$i$Object.freeze($type$.$capital_name$Case);\n\n");
}

void OneofGenerator::generateCase(io::Printer& p) {
  p.Print(variables_,
          "$i$Object.defineProperties($type$.prototype, {\n"
          "$i$  $name$Case: {\n"
          "$i$    get: function() {\n"
          "$i$      return this._oneofs[$index$];\n"
          "$i$    },\n"
          "$i$  },\n"
          "$i$  $name$: {\n"
          "$i$    get: function() {\n"
          "$i$      switch (this._oneofs[$index$]) {\n");
  for (int i = 0; i < t_->field_count(); ++i) {
    auto fname = t_->field(i)->name();
    auto v = variables_;
    v["field_name"] = camelize(fname);
    v["field_capital_name"] = capitalizeAll(fname);
    p.Print(
        v,
        "$i$        case $type$.$capital_name$Case.$field_capital_name$: {\n"
        "$i$          return '$field_name$';\n"
        "$i$        }\n");
  }
  p.Print(variables_,
          "$i$      }\n"
          "$i$    },\n"
          "$i$  },\n"
          "$i$});\n"
          "$i$$type$.prototype.get$capital_name$Case = function() {\n"
          "$i$  return this.$name$Case;\n"
          "$i$};\n"
          "$i$$type$.prototype.get$capital_name$ = function() {\n"
          "$i$  return this.$name$;\n"
          "$i$};\n");
}

void OneofGenerator::generateClear(google::protobuf::io::Printer& p) {
  p.Print(variables_,
          "$i$$type$.prototype.clear$capital_name$ = function() {\n"
          "$i$  switch (this.$name$Case) {\n");
  for (int i = 0; i < t_->field_count(); ++i) {
    auto f = t_->field(i);
    auto v = variables_;
    switch (f->cpp_type()) {
      case FieldDescriptor::CPPTYPE_STRING:
      case FieldDescriptor::CPPTYPE_MESSAGE:
        v["field_name"] = capitalize(f->name());
        v["field_capital_name"] = capitalizeAll(f->name());
        p.Print(
            v,
            "$i$    case $type$.$capital_name$Case.$field_capital_name$: {\n"
            "$i$      this.clear$field_name$();\n"
            "$i$      break;\n"
            "$i$    }\n");
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
          "$i$  }\n"
          "$i$  this._oneofs[$index$] = "
          "$type$.$capital_name$Case.$all_capital_name$_NOT_SET;\n"
          "$i$};\n\n");
}
}
}
