#ifndef GRPC_QML_CREDENTIALS_H
#define GRPC_QML_CREDENTIALS_H

#include "protobuf/qml/common.h"
#include <grpc++/credentials.h>
#include <QObject>

namespace grpc {
namespace qml {

class GRPC_QML_DLLEXPORT Credentials : public QObject {
  Q_OBJECT
public:
  Credentials(QObject* p = nullptr) : QObject(p) {}

  virtual const std::shared_ptr<grpc::Credentials>& raw() const {
    static std::shared_ptr<grpc::Credentials> empty;
    return empty;
  }
};

class GRPC_QML_DLLEXPORT InsecureCredentials : public Credentials {
  Q_OBJECT
public:
  InsecureCredentials(QObject* p = nullptr)
      : Credentials(p), raw_(grpc::InsecureCredentials()) {}

  const std::shared_ptr<grpc::Credentials>& raw() const final { return raw_; }

private:
  std::shared_ptr<grpc::Credentials> raw_;
};
}
}

#endif  // GRPC_QML_CREDENTIALS_H
