#include "protobuf/qml/compiler_util.h"

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

std::string strip_proto(const std::string& fullname) {
  constexpr auto ext = ".proto";
  constexpr auto ext_size = 6;
  auto base_size = fullname.size() - ext_size;
  if (base_size <= 0 || fullname.compare(base_size, ext_size, ext)) {
    throw std::logic_error("Invalid proto file name.");
  }
  return fullname.substr(0, base_size);
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
  ss << strip_proto(t->name()) << ".pb.js";
  return ss.str();
}

std::string generateImportName(const google::protobuf::FileDescriptor* t) {
  std::ostringstream ss;
  ss << "Q__" << strip_proto(t->name()) << "__";
  return ss.str();
}
}
}
