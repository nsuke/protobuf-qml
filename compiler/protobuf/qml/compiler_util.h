#ifndef PROTOBUF_QML_COMPILER_UTIL_H
#define PROTOBUF_QML_COMPILER_UTIL_H

#include <google/protobuf/descriptor.h>
#include <string>

namespace protobuf {
namespace qml {

std::string generateLongName(const google::protobuf::Descriptor* t);

std::string generateLongName(const google::protobuf::EnumValueDescriptor* t);

std::string generateLongName(const google::protobuf::EnumDescriptor* t);

std::string generateFilePath(const google::protobuf::FileDescriptor* t);

std::string generateImportName(const google::protobuf::FileDescriptor* t);

std::string strip_proto(const std::string& fullname);

}
}
#endif  // PROTOBUF_QML_COMPILER_UTIL_H
