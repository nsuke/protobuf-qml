#include "protobuf/qml/util.h"
#include <sstream>

namespace protobuf {
namespace qml {

constexpr auto kCapitalizeOffset = 'a' - 'A';

inline bool is_small_char(char c) {
  return c >= 'a' && c <= 'z';
}

inline char capitalize(char c) {
  return c - kCapitalizeOffset;
}

std::string camelize(const std::string& name) {
  std::ostringstream ss;
  bool capitalizing = false;
  for (auto& c : name) {
    if (capitalizing) {
      capitalizing = false;
      if (is_small_char(c)) {
        ss << capitalize(c);
      } else {
        ss << '_' << c;
      }
    } else {
      if (c == '_') {
        capitalizing = true;
      } else {
        ss << c;
      }
    }
  }
  return ss.str();
}

std::string capitalizeAll(std::string name) {
  for (size_t i = 0; i < name.size(); ++i) {
    auto c = name[i];
    name[i] = is_small_char(c) ? capitalize(c) : c;
  }
  return std::move(name);
}
}
}
