#ifndef GRPC_QML_LOGGING_H
#define GRPC_QML_LOGGING_H

#define ENABLE_MSG_DEBUG 1

#if ENABLE_MSG_DEBUG

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(msg)

#define MSG_DEBUG qCDebug(msg)

#else

namespace grpc {
namespace qml {
struct dev_null {
  template <typename T>
  dev_null& operator<<(T&) {
    return *this;
  }
};
}
}

#define MSG_DEBUG ::grpc::qml::dev_null()

#endif

#endif  // GRPC_QML_LOGGING_H
