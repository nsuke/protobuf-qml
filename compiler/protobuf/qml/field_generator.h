#ifndef PROTOBUF_QML_FIELD_GENERATOR_H
#define PROTOBUF_QML_FIELD_GENERATOR_H

#include "protobuf/qml/compiler_common.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

namespace protobuf {
namespace qml {

class FieldGenerator {
public:
  FieldGenerator(FieldGenerator&& other)
      : t_(other.t_),
        camel_name_(std::move(other.camel_name_)),
        capital_name_(std::move(other.capital_name_)),
        variables_(std::move(other.variables_)) {}
  FieldGenerator(const google::protobuf::FieldDescriptor* t);
  void generateInit(google::protobuf::io::Printer&);
  void generateMerge(google::protobuf::io::Printer&, const std::string& arg);
  void generateProperty(google::protobuf::io::Printer&);
  bool is_oneof() const { return t_->containing_oneof(); }

  PBQML_USE_INDENT

private:
  std::string defaultValue();
  void generateRepeatedProperty(google::protobuf::io::Printer&);
  void generateOptionalProperty(google::protobuf::io::Printer&);
  void messageAssertLength(google::protobuf::io::Printer& p);

  void genSet(google::protobuf::io::Printer& p);
  void genGet(google::protobuf::io::Printer& p);
  void genClear(google::protobuf::io::Printer& p);
  void countMethod(google::protobuf::io::Printer& p);
  void addMethod(google::protobuf::io::Printer& p);

  const google::protobuf::FieldDescriptor* t_;
  bool is_message_;
  bool is_typed_array_ = false;
  const google::protobuf::OneofDescriptor* oneof_;
  std::string camel_name_;
  std::string capital_name_;
  std::map<std::string, std::string> variables_;
};
}
}

#endif  // PROTOBUF_QML_FIELD_GENERATOR_H
