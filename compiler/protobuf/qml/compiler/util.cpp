#include "protobuf/qml/compiler/util.h"

#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <sstream>
#include <stack>
#include <stdexcept>

namespace protobuf {
namespace qml {

template <typename Descriptor>
void appendLongName(std::ostream& o, Descriptor* t) {
  if (!t) {
    throw std::invalid_argument("Descriptor is null");
  }

  std::stack<std::string> names;
  names.push(t->name());
  auto c = t->containing_type();
  if (c) {
    do {
      names.push(c->name());
    } while ((c = c->containing_type()));
  }

  o << names.top();
  names.pop();
  while (!names.empty()) {
    o << "_" << names.top();
    names.pop();
  }
}

std::string generateLongName(const google::protobuf::Descriptor* t) {
  std::ostringstream ss;
  appendLongName(ss, t);
  return ss.str();
}

std::string generateLongName(const google::protobuf::EnumValueDescriptor* t) {
  std::ostringstream ss;
  appendLongName(ss, t->type());
  ss << "." << t->name();
  return ss.str();
}

std::string generateLongName(const google::protobuf::EnumDescriptor* t) {
  std::ostringstream ss;
  appendLongName(ss, t);
  return ss.str();
}

std::string generateFilePath(const google::protobuf::FileDescriptor* t) {
  std::ostringstream ss;
  ss << google::protobuf::compiler::cpp::StripProto(t->name()) << ".pb.js";
  return ss.str();
}

std::string generateImportName(const google::protobuf::FileDescriptor* t) {
  std::ostringstream ss;
  ss << "Q__" << google::protobuf::compiler::cpp::StripProto(t->name()) << "__";
  return ss.str();
}
}
}
