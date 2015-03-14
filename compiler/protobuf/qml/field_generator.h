#ifndef PROTOBUF_QML_FIELD_GENERATOR_H
#define PROTOBUF_QML_FIELD_GENERATOR_H

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <qglobal.h>

namespace protobuf {
namespace qml {

class FieldGenerator {
 public:
  FieldGenerator(FieldGenerator&& other)
      : t_(other.t_),
        camel_name_(std::move(other.camel_name_)),
        capital_name_(std::move(other.capital_name_)) {}
  FieldGenerator(const google::protobuf::FieldDescriptor* t);
  void generateInit(google::protobuf::io::Printer&);
  void generateMerge(google::protobuf::io::Printer&, const std::string& arg);
  void generateProperty(google::protobuf::io::Printer&);

 private:
  void generateRepeatedMessageProperty(google::protobuf::io::Printer&);
  void generateRepeatedProperty(google::protobuf::io::Printer&);
  void generateOptionalMessageProperty(google::protobuf::io::Printer&);
  void generateOptionalProperty(google::protobuf::io::Printer&);

  const google::protobuf::FieldDescriptor* t_;
  std::string camel_name_;
  std::string capital_name_;
};
}
}

#endif  // PROTOBUF_QML_FIELD_GENERATOR_H
