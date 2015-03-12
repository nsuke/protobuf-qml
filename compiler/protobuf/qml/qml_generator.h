#ifndef PROTOBUF_QML_QML_GENERATOR_H
#define PROTOBUF_QML_QML_GENERATOR_H

#include "protobuf/qml/message_generator.h"
#include "protobuf/qml/enum_generator.h"
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

namespace protobuf {
namespace qml {

class QmlGenerator : public google::protobuf::compiler::CodeGenerator {
 public:
  virtual bool Generate(
      const google::protobuf::FileDescriptor* file,
      const std::string& parameter,
      google::protobuf::compiler::GeneratorContext* generator_context,
      std::string* error) const override;
};

class FileGenerator {
 public:
  FileGenerator(const google::protobuf::FileDescriptor* file);

  void generateJsFile(google::protobuf::io::Printer&);
  void generateMessage(google::protobuf::io::Printer&,
                       const google::protobuf::Descriptor*,
                       bool top_level = true);

 private:
  int serializedFileDescriptor(std::string&);

  const google::protobuf::FileDescriptor* file_;
  std::vector<MessageGenerator> message_generators_;
  std::vector<EnumGenerator> enum_generators_;
};
}
}

#endif  // PROTOBUF_QML_QML_GENERATOR_H
