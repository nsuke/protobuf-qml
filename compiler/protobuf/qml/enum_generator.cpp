#include "protobuf/qml/enum_generator.h"
#include "protobuf/qml/compiler_util.h"
#include <sstream>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

void EnumGenerator::generateEnum(io::Printer& p) {
  p.Print(
      "$i$\n"
      "$i$var $name$ = {\n",
      "name", t_->name(), "i", indent_);
  for (int i = 0; i < t_->value_count(); i++) {
    p.Print("$i$  $enum_value_name$: $enum_value$,\n", "enum_value_name",
            t_->value(i)->name(), "enum_value",
            std::to_string(t_->value(i)->number()), "i", indent_);
  }
  p.Print(
      "$i$\n"
      "$i$  toString: function(value) {\n"
      "$i$    switch(value) {\n",
      "i", indent_);
  for (int i = 0; i < t_->value_count(); i++) {
    p.Print("$i$      case $enum_value$: return '$enum_value_name$';\n",
            "enum_value_name", t_->value(i)->name(), "enum_value",
            std::to_string(t_->value(i)->number()), "i", indent_);
  }
  p.Print(
      "$i$    }\n"
      "$i$  },\n"
      "$i$};\n",
      "i", indent_);
}

void EnumGenerator::generateNestedAlias(google::protobuf::io::Printer& p) {
  p.Print("$i$type.$name$ = $name$;\n", "name", t_->name(), "i", indent_);
}
}
}
