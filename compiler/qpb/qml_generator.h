#ifndef QPB_QML_GENERATOR_H
#define QPB_QML_GENERATOR_H

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>

namespace google {
namespace protobuf {
class DescriptorPool;
class Descriptor;
class FileDescriptor;
namespace io {
class Printer;
}
}
}

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
  void generateEnum(google::protobuf::io::Printer&,
                       const google::protobuf::EnumDescriptor*);
  void generateMessage(google::protobuf::io::Printer&,
                       const google::protobuf::Descriptor*);

 private:
  int serializedFileDescriptor(std::string&);

  const google::protobuf::FileDescriptor* file_;
};
}

#endif  // QPB_QML_GENERATOR_H
