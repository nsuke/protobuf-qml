#ifndef PROTOBUF_QML_ONEOF_GENERATOR_H
#define PROTOBUF_QML_ONEOF_GENERATOR_H

#include "protobuf/qml/compiler_common.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

namespace protobuf {
namespace qml {

class OneofGenerator {
public:
  OneofGenerator(const google::protobuf::OneofDescriptor* t);
  void generateMerge(google::protobuf::io::Printer&, const std::string& arg);
  void generateInit(google::protobuf::io::Printer&);
  void generate(google::protobuf::io::Printer&);

  PBQML_USE_INDENT

private:
  void generateCaseEnum(google::protobuf::io::Printer&);
  void generateCase(google::protobuf::io::Printer&);
  void generateClear(google::protobuf::io::Printer&);

  const google::protobuf::OneofDescriptor* t_;
  std::string camel_name_;
  std::string capital_name_;
  std::string all_capital_name_;
  std::map<std::string, std::string> variables_;
};
}
}
#endif  // PROTOBUF_QML_ONEOF_GENERATOR_H
