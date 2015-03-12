#include "protobuf/qml/field_generator.h"
#include "protobuf/qml/enum_generator.h"
#include <google/protobuf/stubs/strutil.h>
#include <QDebug>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

void FieldGenerator::generateInit(io::Printer& p) {
  p.Print("  this._fields[$index$] = (values && values.$name$)",
          "index",
          SimpleItoa(t_->index()),
          "name",
          t_->name());
  switch (t_->cpp_type()) {
#define PRINT_DEFAULT_VALUE(type1, type2, prefix)                            \
  case FieldDescriptor::CPPTYPE_##type1:                                     \
    p.Print(                                                                 \
        " || $default$;\n", "default", prefix(t_->default_value_##type2())); \
    break;
    PRINT_DEFAULT_VALUE(INT32, int32, SimpleItoa);
    PRINT_DEFAULT_VALUE(INT64, int64, SimpleItoa);
    PRINT_DEFAULT_VALUE(UINT32, uint32, SimpleItoa);
    PRINT_DEFAULT_VALUE(UINT64, uint64, SimpleItoa);
    PRINT_DEFAULT_VALUE(FLOAT, float, SimpleFtoa);
    PRINT_DEFAULT_VALUE(DOUBLE, double, SimpleFtoa);
    // PRINT_DEFAULT_VALUE(BOOL, bool, );
    PRINT_DEFAULT_VALUE(ENUM, enum, EnumGenerator::generateLongName);
    PRINT_DEFAULT_VALUE(STRING, string, );
#undef PRINT_DEFAULT_VALUE
    case FieldDescriptor::CPPTYPE_BOOL:
      p.Print(" || $default$;\n",
              "default",
              t_->default_value_bool() ? "true" : "false");
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      p.Print(";\n");
      break;
    default:
      qDebug() << t_->cpp_type();
      Q_ASSERT(false);
  }
}

void FieldGenerator::generateProperty(google::protobuf::io::Printer& p) {
  p.Print(
      "  $name$: {\n"
      "    get: function() {\n"
      "      return this._fields[$index$];\n"
      "    },\n"
      "    set: function(value) {\n"
      "      this._fields[$index$] = value;\n"
      "    },\n"
      "  },\n",
      "name",
      t_->name(),
      "index",
      SimpleItoa(t_->index()));
}
}
}
