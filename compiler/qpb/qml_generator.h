#ifndef QPB_QML_GENERATOR_H
#define QPB_QML_GENERATOR_H

#include "qpb/generators.h"
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

namespace qpb {

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
  std::vector<std::unique_ptr<MessageGenerator>> message_generators_;
  std::vector<std::unique_ptr<EnumGenerator>> enum_generators_;
};
}

#endif  // QPB_QML_GENERATOR_H
