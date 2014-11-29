#include "qpb/generators.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <QByteArray>

#include <memory>
#include <stdexcept>

namespace qpb {

using namespace google::protobuf;

void StructureUtil::header(io::Printer& p,
                           bool top_level,
                           const std::string& name) {
  if (top_level) {
    p.Print(
        "\n"
        "var $name$ = {\n",
        "name",
        name);
  } else {
    p.Print(
        "\n"
        "$name$: {\n",
        "name",
        name);
  }
}

void StructureUtil::footer(io::Printer& p, bool top_level) {
  if (top_level) {
    p.Print("};\n");
  } else {
    p.Print("},\n");
  }
}

MessageGenerator::MessageGenerator(const Descriptor* t, bool top_level)
    : t_(t), top_level_(top_level) {
  if (!t) {
    throw std::invalid_argument("Null descriptor");
  }
  for (int i = 0; i < t_->nested_type_count(); i++) {
    message_generators_.emplace_back(
        new MessageGenerator(t_->nested_type(i), false));
  }
  for (int i = 0; i < t_->enum_type_count(); i++) {
    enum_generators_.emplace_back(new EnumGenerator(t_->enum_type(i), false));
  }
}

void MessageGenerator::generateMessageInit(io::Printer& p) {
  p.Print(
      "        this.$message_name$ = "
      "this.file.messageType($message_index$);\n",
      "message_name",
      t_->name(),
      "message_index",
      SimpleItoa(t_->index()));
}

void MessageGenerator::generateMessage(io::Printer& p) {
  StructureUtil::header(p, top_level_, t_->name());
  p.Print(
      "  callbacks: [],\n"
      "  initialized: false,\n"
      "  initOnce: function() {\n"
      "    if(init.once() && !this.initialized) {\n"
      "      var that = this;\n"
      "      init.$message_name$.serializeCompleted.connect(function(k, err) "
      "{\n"
      "        if(!that.callbacks[k](err))\n"
      "          that.callbacks.splice(k, 1);\n"
      "      });\n"
      "      init.$message_name$.parseCompleted.connect(function(k, msg, err) "
      "{\n"
      "        if(!that.callbacks[k](msg, err))\n"
      "          that.callbacks.splice(k, 1);\n"
      "      });\n"
      "      this.initialized = true;\n"
      "    }\n"
      "    return this.initialized;\n"
      "  },\n"
      "  parse: function(input, callback) {\n"
      "    if(!this.initOnce()) return;\n"
      "    if(typeof callback == 'undefined') {\n"
      "      return init.$message_name$.parse(input);\n"
      "    } else {\n"
      "      var k = init.$message_name$.nextKey();\n"
      "      if(!k) return;\n"
      "      this.callbacks[k] = callback;\n"
      "      return init.$message_name$.parseAsync(k, input);\n"
      "    }\n"
      "  },\n"
      "  serialize: function(output, value, callback) {\n"
      "    if(!this.initOnce()) return;\n"
      "    if(typeof callback == 'undefined') {\n"
      "      return init.$message_name$.serialize(output, value);\n"
      "    } else {\n"
      "      var k = init.$message_name$.nextKey();\n"
      "      if(!k) return;\n"
      "      this.callbacks[k] = callback;\n"
      "      return init.$message_name$.serializeAsync(k, output, value);\n"
      "    }\n"
      "  },\n",
      "message_name",
      t_->name());
  for (auto& g : enum_generators_) {
    g->generateEnum(p);
  }
  for (auto& g : message_generators_) {
    g->generateMessage(p);
  }
  StructureUtil::footer(p, top_level_);
}

void EnumGenerator::generateEnum(io::Printer& p) {
  StructureUtil::header(p, top_level_, t_->name());
  for (int i = 0; i < t_->value_count(); i++) {
    p.Print("  $enum_value_name$: $enum_value$,\n",
            "enum_value_name",
            t_->value(i)->name(),
            "enum_value",
            SimpleItoa(t_->value(i)->number()));
  }
  p.Print(
      "\n"
      "  toString: function(value) {\n"
      "    switch(value) {\n");
  for (int i = 0; i < t_->value_count(); i++) {
    p.Print("      case $enum_value$: return '$enum_value_name$';\n",
            "enum_value_name",
            t_->value(i)->name(),
            "enum_value",
            SimpleItoa(t_->value(i)->number()));
  }
  p.Print(
      "    }\n"
      "  },\n");
  StructureUtil::footer(p, top_level_);
}
}
