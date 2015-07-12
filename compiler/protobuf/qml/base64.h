#ifndef PROTOBUF_QML_BASE64_H
#define PROTOBUF_QML_BASE64_H

#include "protobuf/qml/compiler_common.h"
#include <string>

#if PBQML_COMPILER_HAS_QT

#include <QByteArray>

namespace protobuf {
namespace qml {

inline QByteArray base64Buffer(size_t size) {
  return QByteArray(size, ' ');
}

inline std::string toBase64(QByteArray& buf) {
  return buf.toBase64().toStdString();
}
}
}

#else  // PBQML_COMPILER_HAS_QT

#include <vector>

namespace protobuf {
namespace qml {

inline std::vector<unsigned char> base64Buffer(size_t size) {
  return std::vector<unsigned char>(size);
}

// TODO: Provide default implementation
std::string toBase64(std::vector<unsigned char>& buf);
}
}
#endif  // PBQML_COMPILER_HAS_QT
#endif  // PROTOBUF_QML_BASE64_H
