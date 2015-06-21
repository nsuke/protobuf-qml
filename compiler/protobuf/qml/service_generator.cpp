#include "protobuf/qml/service_generator.h"

namespace protobuf {
namespace qml {

ServiceGenerator::ServiceGenerator(const google::protobuf::ServiceDescriptor* t)
    : t_(t) {
  if (t_) {
    for (int i = 0; i < t_->method_count(); ++i) {
      auto method = t_->method(i);
      IOType in(method->input_type());
      IOType o(method->output_type());
      method_generators_.emplace_back(method, in.qualified_name,
                                      o.qualified_name);
      imports_.insert(std::move(in));
      imports_.insert(std::move(o));
    }
  }
}

void ServiceGenerator::generateQmlFile(google::protobuf::io::Printer& p) {
  if (!t_) throw std::logic_error("Cannot generate for null descriptor.");
  p.Print(
      "import QtQuick 2.2\n"
      "import Protobuf 1.0 as PB\n"
      "\n");

  for (auto& import : imports_) {
    p.Print("import '$file_name$' as $capital_name$\n", "file_name",
            import.file_path, "capital_name", import.import_name);
  }

  p.Print(
      "\n"
      "Item {\n"
      "  id: root\n"
      "  property var channel\n");

  for (auto& g : method_generators_) {
    p.Print("\n");
    g.generateMethod(p);
  }

  p.Print("}\n");
}

MethodGenerator::MethodGenerator(const google::protobuf::MethodDescriptor* t,
                                 const std::string& input_type_name,
                                 const std::string& output_type_name)
    : t_(t) {
  auto w = t_->client_streaming();
  auto r = t_->server_streaming();

  auto capital_name = t_->name();
  auto camel_name = uncapitalizeFirstLetter(t_->name());

  variables = {
      {"camel_name", camel_name},
      {"capital_name", capital_name},
      {"service_name", t_->service()->name()},
      {"input_type", input_type_name},
      {"output_type", output_type_name},
  };
  if (!w && !r) {
    variables.insert(std::make_pair("method_type", "Unary"));
  } else if (w && !r) {
    variables.insert(std::make_pair("method_type", "Writer"));
  } else {
    // not implemented
  }
}

void MethodGenerator::generateMethod(google::protobuf::io::Printer& p) {
  auto w = t_->client_streaming();
  auto r = t_->server_streaming();
  if (!w && !r) {
    generateUnaryMethod(p);
  } else if (w && !r) {
    generateWriterMethod(p);
  } else {
    // not implemented
  }
  generateMethodElement(p);
}

void MethodGenerator::generateMethodElement(google::protobuf::io::Printer& p) {
  p.Print(variables,
          "  PB.$method_type$Method {\n"
          "    id: $camel_name$Method\n"
          "    methodName: '/$service_name$/$capital_name$'\n"
          "    channel: root.channel || null\n"
          "    readDescriptor: $output_type$.descriptor\n"
          "    writeDescriptor: $input_type$.descriptor\n"
          "  }\n");
}

void MethodGenerator::generateUnaryMethod(google::protobuf::io::Printer& p) {
  p.Print(variables,
          "  function $camel_name$(data, callback, timeout) {\n"
          "    return $camel_name$Method.call(new $input_type$(data)._raw, "
          "function(data, err) {\n"
          "      callback(new $output_type$(data), err);\n"
          "    }, timeout);\n"
          "  }\n");
}

void MethodGenerator::generateWriterMethod(google::protobuf::io::Printer& p) {
  p.Print(variables,
          "  function $camel_name$(callback, timeout) {\n"
          "    var call = $camel_name$Method.call(function(data, err) {\n"
          "      callback(new $output_type$(data), err);\n"
          "    }, timeout);\n"
          "    call.write = function(data, callback) {\n"
          "      return call._write(new $input_type$(data)._raw);\n"
          "    };\n"
          "    return call;\n"
          "  }\n");
}
}
}
