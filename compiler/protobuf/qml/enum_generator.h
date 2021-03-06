#ifndef PROTOBUF_QML_ENUM_GENERATOR_H
#define PROTOBUF_QML_ENUM_GENERATOR_H

#include "protobuf/qml/message_generator.h"
#include "protobuf/qml/compiler_util.h"
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.h>

namespace protobuf {
namespace qml {

class EnumGenerator {
public:
  EnumGenerator(const google::protobuf::EnumDescriptor* t,
                std::string indent = "")
      : t_(t), indent_(indent) {
    if (!t) {
      throw std::invalid_argument("Null descriptor");
    }
  }
  void generateEnum(google::protobuf::io::Printer& p);
  void generateNestedAlias(google::protobuf::io::Printer& p);

private:
  const google::protobuf::EnumDescriptor* t_;
  std::string indent_;
};
}
}

#endif  // PROTOBUF_QML_ENUM_GENERATOR_H
