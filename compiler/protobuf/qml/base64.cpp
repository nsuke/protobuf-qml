#include "protobuf/qml/base64.h"

namespace protobuf {
namespace qml {

std::string toBase64(std::vector<unsigned char>& buf) {
  constexpr auto table =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

  auto size = buf.size();
  std::string result;
  result.reserve(((size - 1) / 3 + 1) * 4);
  size_t i = 0;
  for (; i < size - 2; i += 3) {
    result.push_back(table[(static_cast<int>(buf[i]) >> 2) & 0x3f]);
    result.push_back(table[((static_cast<int>(buf[i]) << 4) & 0x30) +
                           ((static_cast<int>(buf[i + 1]) >> 4) & 0xf)]);
    result.push_back(table[((static_cast<int>(buf[i + 1]) << 2) & 0x3c) +
                           ((static_cast<int>(buf[i + 2]) >> 6) & 0x3)]);
    result.push_back(table[((static_cast<int>(buf[i + 2])) & 0x3f)]);
  }
  auto mod = size - i;
  if (mod == 1) {
    result.push_back(table[(static_cast<int>(buf[i]) >> 2) & 0x3f]);
    result.push_back(table[((static_cast<int>(buf[i]) << 4) & 0x30)]);
    result.push_back('=');
    result.push_back('=');
  } else if (mod == 2) {
    result.push_back(table[(static_cast<int>(buf[i]) >> 2) & 0x3f]);
    result.push_back(table[((static_cast<int>(buf[i]) << 4) & 0x30) +
                           ((static_cast<int>(buf[i + 1]) >> 4) & 0xf)]);
    result.push_back(table[((static_cast<int>(buf[i + 1]) << 2) & 0x3c)]);
    result.push_back('=');
  }
  return std::move(result);
}
}
}
