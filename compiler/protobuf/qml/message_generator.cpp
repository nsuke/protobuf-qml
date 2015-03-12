#include "protobuf/qml/message_generator.h"
#include <google/protobuf/stubs/strutil.h>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

MessageGenerator::MessageGenerator(const Descriptor* t,
                                   const std::string prefix)
    : t_(t), name_(t ? (prefix + t->name()) : "") {
  if (!t) {
    throw std::invalid_argument("Null descriptor");
  }
  for (int i = 0; i < t_->nested_type_count(); i++) {
    message_generators_.emplace_back(t_->nested_type(i), name_ + "_");
  }
  for (int i = 0; i < t_->enum_type_count(); i++) {
    enum_generators_.emplace_back(t_->enum_type(i));
  }
  for (int i = 0; i < t_->field_count(); ++i) {
    field_generators_.emplace_back(t_->field(i));
  }
}

void MessageGenerator::generateMessageConstructor(io::Printer& p) {
  p.Print("var $message_name$ = function(values) {\n", "message_name", name_);
  for (auto& g : field_generators_) {
    g.generateInit(p);
  }
  p.Print("};\n\n");
}

void MessageGenerator::generateMessagePrototype(io::Printer& p) {
  p.Print(
      "$message_name$.prototype = new "
      "Protobuf.Message(_file.descriptor.messageType($message_index$));\n\n",
      "message_name",
      name_,
      "message_index",
      SimpleItoa(t_->index()));
}

void MessageGenerator::generateMessageProperties(io::Printer& p) {
  p.Print("Object.defineProperties($message_name$.prototype, {\n",
          "message_name",
          name_);
  for (auto& g : field_generators_) {
    g.generateProperty(p);
  }
  p.Print("});\n\n");
  for (auto& g : enum_generators_) {
    g.generateEnum(p);
  }
  for (auto& g : message_generators_) {
    g.generateMessage(p);
  }
}
}
}
