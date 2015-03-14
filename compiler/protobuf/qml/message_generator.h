#ifndef PROTOBUF_QML_MESSAGE_GENERATOR_H
#define PROTOBUF_QML_MESSAGE_GENERATOR_H

#include "protobuf/qml/field_generator.h"
#include "protobuf/qml/enum_generator.h"
#include <string>

namespace protobuf {
namespace qml {

class EnumGenerator;

class MessageGenerator {
 public:
  MessageGenerator(const google::protobuf::Descriptor* t = nullptr);

  MessageGenerator(MessageGenerator&& other) : t_(other.t_) {
    name_ = std::move(other.name_);
    other.t_ = nullptr;
    message_generators_ = std::move(other.message_generators_);
    enum_generators_ = std::move(other.enum_generators_);
    field_generators_ = std::move(other.field_generators_);
  }

  void generateMessage(google::protobuf::io::Printer& p) {
    generateMessageConstructor(p);
    generateMessagePrototype(p);
    generateMessageProperties(p);
  }

 private:
  MessageGenerator(const MessageGenerator&) = delete;
  MessageGenerator& operator=(const MessageGenerator&) = delete;

  void generateMessageConstructor(google::protobuf::io::Printer& p);
  void generateMessagePrototype(google::protobuf::io::Printer& p);
  void generateMessageProperties(google::protobuf::io::Printer& p);

  std::string name_;
  const google::protobuf::Descriptor* t_;
  std::vector<MessageGenerator> message_generators_;
  std::vector<EnumGenerator> enum_generators_;
  std::vector<FieldGenerator> field_generators_;
};
}
}
#endif  // PROTOBUF_QML_MESSAGE_GENERATOR_H
