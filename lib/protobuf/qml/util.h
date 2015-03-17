#ifndef PROTOBUF_QML_UTIL_H
#define PROTOBUF_QML_UTIL_H

#include <string>

namespace protobuf {
namespace qml {

std::string camelize(const std::string& name);
std::string capitalizeAll(std::string name);
inline std::string capitalizeFirstLetter(std::string camel_name) {
  if (!camel_name.empty() && camel_name[0] <= 'z' && camel_name[0] >= 'a') {
    camel_name[0] += 'A' - 'a';
  }
  return std::move(camel_name);
}
inline std::string capitalize(const std::string& name) {
  return capitalizeFirstLetter(camelize(name));
}
}
}
#endif  // PROTOBUF_QML_UTIL_H
