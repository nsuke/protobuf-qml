#ifndef QPB_GENERATORS_H
#define QPB_GENERATORS_H

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <memory>
#include <string>
#include <vector>

namespace qpb {

class StructureUtil {
 public:
  static void header(google::protobuf::io::Printer&,
                     bool top_level,
                     const std::string& name);
  static void footer(google::protobuf::io::Printer&, bool top_level);
};

class EnumGenerator {
 public:
  EnumGenerator(const google::protobuf::EnumDescriptor* t) : t_(t) {
    if (!t) {
      throw std::invalid_argument("Null descriptor");
    }
  }
  void generateEnum(google::protobuf::io::Printer& p, bool top_level = true);

 private:
  const google::protobuf::EnumDescriptor* t_;
};

class ExtensionGenerator {
 public:
};

class MessageGenerator {
 public:
  MessageGenerator(const google::protobuf::Descriptor* t);
  void generateMessage(google::protobuf::io::Printer& p, bool top_level = true);
  void generateMessageInit(google::protobuf::io::Printer& p);

 private:
  MessageGenerator(const MessageGenerator&) = delete;
  MessageGenerator& operator=(const MessageGenerator&) = delete;

  const google::protobuf::Descriptor* t_;
  int index_;
  std::vector<std::unique_ptr<MessageGenerator>> message_generators_;
  std::vector<std::unique_ptr<EnumGenerator>> enum_generators_;
  std::vector<std::unique_ptr<ExtensionGenerator>> extension_generators_;
};
}

#endif  // QPB_GENERATORS_H
