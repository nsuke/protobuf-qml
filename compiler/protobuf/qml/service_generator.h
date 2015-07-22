#ifndef PROTOBUF_QML_SERVICE_GENERATOR_H
#define PROTOBUF_QML_SERVICE_GENERATOR_H

#include "protobuf/qml/util.h"
#include "protobuf/qml/compiler_util.h"

#include <google/protobuf/io/printer.h>
#include <unordered_set>

namespace protobuf {
namespace qml {

struct IOType {
  std::string file_path;
  std::string import_name;
  std::string qualified_name;

  explicit IOType(const google::protobuf::Descriptor* t) {
    auto file = t->file();
    auto path = file->name();
    file_path =
        path.substr(0, path.size() - std::string("proto").size()) + "pb.js";
    auto pos = path.find_last_of("/");
    auto filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
    import_name = capitalize(strip_proto(filename));
    qualified_name = import_name + "." + t->name();
  }

  bool operator==(const IOType& other) const {
    return file_path.compare(other.file_path) == 0;
  }
};
}
}

namespace std {
template <>
struct hash<protobuf::qml::IOType> {
  size_t operator()(const protobuf::qml::IOType& x) const {
    return hash<std::string>()(x.file_path);
  }
};
}

namespace protobuf {
namespace qml {
class MethodGenerator;
class ServiceGenerator {
public:
  explicit ServiceGenerator(const google::protobuf::ServiceDescriptor* t);

  void generateClientQmlFile(google::protobuf::io::Printer& p);
  void generateServerQmlFile(google::protobuf::io::Printer& p);

private:
  void generateImports(google::protobuf::io::Printer& p);
  const google::protobuf::ServiceDescriptor* t_;
  std::unordered_set<IOType> imports_;
  std::vector<MethodGenerator> method_generators_;
};

class MethodGenerator {
public:
  MethodGenerator(const google::protobuf::MethodDescriptor* t,
                  const std::string& input_type_name,
                  const std::string& output_type_name);

  void generateMethod(google::protobuf::io::Printer& p);
  void generateServerMethod(google::protobuf::io::Printer& p);
  std::string& name() { return variables["camel_name"]; }
  bool is_unary() {
    auto w = t_->client_streaming();
    auto r = t_->server_streaming();
    return !w && !r;
  }

private:
  void generateMethodElement(google::protobuf::io::Printer& p);
  void withPayload(google::protobuf::io::Printer& p);
  void withoutPayload(google::protobuf::io::Printer& p);

  const google::protobuf::MethodDescriptor* t_;
  std::map<std::string, std::string> variables;
};
}
}

#endif  // PROTOBUF_QML_SERVICE_GENERATOR_H
