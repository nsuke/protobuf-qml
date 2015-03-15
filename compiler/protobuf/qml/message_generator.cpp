#include "protobuf/qml/message_generator.h"
#include <google/protobuf/stubs/strutil.h>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

MessageGenerator::MessageGenerator(const Descriptor* t)
    : t_(t), name_(t ? generateLongName(t) : "") {
  if (!t) {
    throw std::invalid_argument("Null descriptor");
  }
  for (int i = 0; i < t_->nested_type_count(); i++) {
    message_generators_.emplace_back(t_->nested_type(i));
  }
  for (int i = 0; i < t_->enum_type_count(); i++) {
    enum_generators_.emplace_back(t_->enum_type(i));
  }
  for (int i = 0; i < t_->field_count(); ++i) {
    field_generators_.emplace_back(t_->field(i));
  }
}

void MessageGenerator::generateMessageConstructor(io::Printer& p) {
  p.Print(
      "var $message_name$ = (function() {\n"
      "  var FIELD = 0;\n"
      "  var ONEOF = 1;\n"
      "  var constructor = function(values) {\n"
      "    this._raw = [new Array($field_count$), new Array($oneof_count$)];\n\n"
      "    this._mergeFromRawArray = function(rawArray) {\n"
      "      if (rawArray && rawArray instanceof Array) {\n",
      "message_name",
      name_,
      "field_count",
      SimpleItoa(t_->field_count()),
      "oneof_count",
      SimpleItoa(t_->oneof_decl_count() + 1));
  for (auto& g : field_generators_) {
    g.generateMerge(p, "rawArray");
  }
  p.Print(
      "      };\n"
      "    };\n\n");
  for (auto& g : field_generators_) {
    g.generateInit(p);
  }
  p.Print(
      "    Object.seal(this);\n"
      "    if (values instanceof $message_name$) {\n"
      "      this._mergeFromRawArray(values._raw);\n"
      "    } else {\n"
      "      for (var k in values) {\n"
      "        this[k](values[k]);\n"
      "      }\n"
      "    }\n"
      "  };\n\n"
      "  Protobuf.Boilerplate.createMessageType(constructor, "
      "_file.descriptor.messageType($message_index$));\n",
      "message_name",
      name_,
      "message_index",
      SimpleItoa(t_->index()));
  for (auto& g : field_generators_) {
    g.generateProperty(p);
  }
  p.Print(
      "  return constructor;\n"
      "})();\n\n");
}

void MessageGenerator::generateMessagePrototype(io::Printer& p) {
  p.Print(
      "Protobuf.Message.createMessageType($message_name$, "
      "_file.descriptor.messageType($message_index$));\n\n",
      "message_name",
      name_,
      "message_index",
      SimpleItoa(t_->index()));
}

void MessageGenerator::generateMessageProperties(io::Printer& p) {
  // p.Print("Object.defineProperties($message_name$.prototype, {\n",
  //         "message_name",
  //         name_);
  // p.Print("});\n\n");
  for (auto& g : enum_generators_) {
    g.generateEnum(p);
  }
  for (auto& g : message_generators_) {
    g.generateMessage(p);
  }
}
}
}
