#include "protobuf/qml/enum_generator.h"
#include "protobuf/qml/compiler_util.h"
#include <sstream>

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
            std::to_string(t_->value(i)->number()));
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
            std::to_string(t_->value(i)->number()));
  }
  p.Print(
      "    }\n"
      "  },\n");
  p.Print("};\n");
}

}
}
