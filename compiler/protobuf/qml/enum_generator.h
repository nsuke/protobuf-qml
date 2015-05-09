#ifndef PROTOBUF_QML_ENUM_GENERATOR_H
#define PROTOBUF_QML_ENUM_GENERATOR_H

#include "protobuf/qml/message_generator.h"
#include "protobuf/qml/compiler/util.h"
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.h>

namespace protobuf {
namespace qml {

class EnumGenerator {
public:
  EnumGenerator(const google::protobuf::EnumDescriptor* t)
      : t_(t), long_name_(t ? generateLongName(t) : "") {
    if (!t) {
      throw std::invalid_argument("Null descriptor");
    }
  }
  void generateEnum(google::protobuf::io::Printer& p);

private:
  const google::protobuf::EnumDescriptor* t_;
  std::string long_name_;
};
}
}

#endif  // PROTOBUF_QML_ENUM_GENERATOR_H
