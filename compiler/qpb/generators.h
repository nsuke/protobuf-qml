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
  EnumGenerator(const google::protobuf::EnumDescriptor* t,
                bool top_level = true)
      : t_(t), top_level_(top_level) {
    if (!t) {
      throw std::invalid_argument("Null descriptor");
    }
  }
  void generateEnum(google::protobuf::io::Printer& p);

 private:
  const google::protobuf::EnumDescriptor* t_;
  bool top_level_;
};

class MessageGenerator {
 public:
  MessageGenerator(const google::protobuf::Descriptor* t,
                   bool top_level = true);
  void generateMessage(google::protobuf::io::Printer& p);
  void generateMessageInit(google::protobuf::io::Printer& p);

 private:
  MessageGenerator(const MessageGenerator&) = delete;
  MessageGenerator& operator=(const MessageGenerator&) = delete;

  const google::protobuf::Descriptor* t_;
  bool top_level_;
  std::vector<std::unique_ptr<MessageGenerator>> message_generators_;
  std::vector<std::unique_ptr<EnumGenerator>> enum_generators_;
};
}

#endif  // QPB_GENERATORS_H
