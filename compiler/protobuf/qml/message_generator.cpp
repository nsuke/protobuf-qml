#include "protobuf/qml/message_generator.h"
#include "protobuf/qml/util.h"

namespace protobuf {
namespace qml {

using namespace google::protobuf;

MessageGenerator::MessageGenerator(const Descriptor* t, std::string indent)
    : t_(t), name_(t ? t->name() : ""), indent_(indent) {
  if (!t) {
    throw std::invalid_argument("Null descriptor");
  }

  message_generators_.reserve(t_->nested_type_count());
  for (int i = 0; i < t_->nested_type_count(); i++) {
    message_generators_.emplace_back(t_->nested_type(i), indent + "  ");
  }

  enum_generators_.reserve(t_->enum_type_count());
  for (int i = 0; i < t_->enum_type_count(); i++) {
    enum_generators_.emplace_back(t_->enum_type(i), indent + "  ");
  }

  field_generators_.reserve(t_->field_count());
  for (int i = 0; i < t_->field_count(); ++i) {
    field_generators_.emplace_back(t_->field(i));
  }
}

void MessageGenerator::generateMessageConstructor(io::Printer& p) {
  // For some reason, oneof_decl_count sometimes reports wrong value in the
  // constructor. So we initialize it here as a dirty work around.
  if (oneof_generators_.empty() && t_->oneof_decl_count() > 0) {
    oneof_generators_.reserve(t_->oneof_decl_count());
    for (int i = 0; i < t_->oneof_decl_count(); ++i) {
      oneof_generators_.emplace_back(t_->oneof_decl(i));
    }
  }

  p.Print(
      "$i$var $message_name$ = (function() {\n"
      "$i$  var type = function(values) {\n"
      "$i$    this._fields = new Array($field_count$);\n"
      "$i$    this._oneofs = new Uint32Array($oneof_count$);\n"
      "$i$    this._raw = [this._fields, this._oneofs.buffer];\n",
      "message_name", name_, "field_count", std::to_string(t_->field_count()),
      "oneof_count", std::to_string(t_->oneof_decl_count()), "i", indent_);
  for (auto& g : field_generators_) {
    g.indent(indent_ + "  ");
    g.generateInit(p);
  }
  for (auto& g : oneof_generators_) {
    g.indent(indent_ + "  ");
    g.generateInit(p);
  }

  p.Print(
      // Sealing provides better coding error detection but degrades performance
      // by 100% on benchmarks.
      // "    Object.seal(this);\n"
      "$i$    if (values instanceof $message_name$) {\n"
      "$i$      this._mergeFromRawArray(values._raw);\n"
      "$i$    } else if (values instanceof Array) {\n"
      "$i$      this._mergeFromRawArray(values);\n"
      "$i$    } else {\n"
      "$i$      for (var k in values) {\n"
      "$i$        if (this[k] instanceof Function) {\n"
      "$i$          this[k](values[k]);\n"
      "$i$        } else {\n"
      "$i$          this[k] = values[k];\n"
      "$i$        }\n"
      "$i$      }\n"
      "$i$    }\n"
      "$i$  };\n\n",
      "message_name", name_, "message_index", std::to_string(t_->index()), "i",
      indent_);

  for (auto& g : enum_generators_) {
    g.generateEnum(p);
    g.generateNestedAlias(p);
  }
  for (auto& g : message_generators_) {
    g.generateMessage(p);
    g.generateNestedAlias(p);
  }

  p.Print(
      "$i$  Protobuf.Message.createMessageType(type, "
      "_file.descriptor.messageType($message_index$));\n\n",
      "message_name", name_, "message_index", std::to_string(t_->index()), "i",
      indent_);

  p.Print(
      "$i$  type.prototype._mergeFromRawArray = function(rawArray) {\n"
      "$i$    if (rawArray && rawArray instanceof Array) {\n"
      "$i$      var oneofs = new Uint32Array(rawArray[1]);\n"
      "$i$      var field;\n",
      "message_name", name_, "field_count", std::to_string(t_->field_count()),
      "oneof_count", std::to_string(t_->oneof_decl_count()), "i", indent_);
  for (auto& g : field_generators_) {
    if (!g.is_oneof()) {
      g.indent(indent_ + "      ");
      g.generateMerge(p, "rawArray");
    }
  }
  for (auto& g : oneof_generators_) {
    g.indent(indent_ + "      ");
    g.generateMerge(p, "rawArray");
  }
  p.Print(
      "$i$    }\n"
      "$i$  };\n\n",
      "i", indent_);

  for (auto& g : oneof_generators_) {
    g.indent(indent_ + "  ");
    g.generate(p);
  }
  for (auto& g : field_generators_) {
    g.indent(indent_ + "  ");
    g.generateProperty(p);
  }
  p.Print(
      "$i$  return type;\n"
      "$i$})();\n\n",
      "i", indent_);
}

void MessageGenerator::generateMessagePrototype(io::Printer& p) {
  p.Print(
      "$i$Protobuf.Message.createMessageType($message_name$, "
      "_file.descriptor.messageType($message_index$));\n\n",
      "message_name", name_, "message_index", std::to_string(t_->index()), "i",
      indent_);
}

void MessageGenerator::generateNestedAlias(google::protobuf::io::Printer& p) {
  p.Print("$i$type.$name$ = $name$;\n", "name", t_->name(), "i", indent_);
}
}
}
