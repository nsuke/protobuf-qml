#include "protobuf/qml/enum_generator.h"
#include <google/protobuf/stubs/strutil.h>
#include <sstream>
#include <stack>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

void EnumGenerator::generateEnum(io::Printer& p) {
  p.Print(
      "\n"
      "var $name$ = {\n",
      "name",
      long_name_);
  for (int i = 0; i < t_->value_count(); i++) {
    p.Print("  $enum_value_name$: $enum_value$,\n",
            "enum_value_name",
            t_->value(i)->name(),
            "enum_value",
            SimpleItoa(t_->value(i)->number()));
  }
  p.Print(
      "\n"
      "  toString: function(value) {\n"
      "    switch(value) {\n");
  for (int i = 0; i < t_->value_count(); i++) {
    p.Print("      case $enum_value$: return '$enum_value_name$';\n",
            "enum_value_name",
            t_->value(i)->name(),
            "enum_value",
            SimpleItoa(t_->value(i)->number()));
  }
  p.Print(
      "    }\n"
      "  },\n");
  p.Print("};\n");
}

void appendLongName(std::ostream& o, const google::protobuf::EnumDescriptor* t) {
  if (!t) {
    throw std::invalid_argument("EnumDescriptor is null");
  }

  std::stack<std::string> names;
  names.push(t->name());
  auto c = t->containing_type();
  if (c) {
    do {
      names.push(c->name());
    } while ((c = c->containing_type()));
  }

  o << names.top();
  names.pop();
  while (!names.empty()) {
    o << "_" << names.top();
    names.pop();
  }
}

std::string EnumGenerator::generateLongName(const google::protobuf::EnumValueDescriptor* t) {
  std::ostringstream ss;
  appendLongName(ss, t->type());
  ss << "." << t->name();
  return ss.str();
}

std::string EnumGenerator::generateLongName(const google::protobuf::EnumDescriptor* t) {
  std::ostringstream ss;
  appendLongName(ss, t);
  return ss.str();
}
}
}
