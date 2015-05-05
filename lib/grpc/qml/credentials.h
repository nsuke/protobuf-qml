#ifndef GRPC_QML_CREDENTIALS_H
#define GRPC_QML_CREDENTIALS_H

#include <QObject>
#include "protobuf/qml/common.h"

namespace grpc {
namespace qml {

class GRPC_QML_DLLEXPORT Credentials : public QObject {
  Q_OBJECT
 public:
  Credentials(QObject* p = 0) : QObject(p) {}
};

class GRPC_QML_DLLEXPORT InsecureCredentials : public Credentials {
  Q_OBJECT
 public:
  InsecureCredentials(QObject* p = 0) : Credentials(p) {}
};
}
}

#endif  // GRPC_QML_CREDENTIALS_H
