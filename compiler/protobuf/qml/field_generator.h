#ifndef PROTOBUF_QML_FIELD_GENERATOR_H
#define PROTOBUF_QML_FIELD_GENERATOR_H

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <qglobal.h>

namespace protobuf {
namespace qml {

class FieldGenerator {
 public:
  FieldGenerator(FieldGenerator&& other) : t_(other.t_) {}
  FieldGenerator(const google::protobuf::FieldDescriptor* t) : t_(t) {}
  void generateInit(google::protobuf::io::Printer&);
  void generateProperty(google::protobuf::io::Printer&);

 private:
  const google::protobuf::FieldDescriptor* t_;
};
}
}

#endif  // PROTOBUF_QML_FIELD_GENERATOR_H
