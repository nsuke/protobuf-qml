#ifndef GRPC_QML_SERVER_CREDENTIALS_H
#define GRPC_QML_SERVER_CREDENTIALS_H

#include <QObject>
#include <grpc++/server_credentials.h>

namespace grpc {
namespace qml {

class ServerCredentials : public QObject {
  Q_OBJECT

public:
  explicit ServerCredentials(QObject* p = nullptr) : QObject(p) {}
  virtual ~ServerCredentials() {}
  virtual const std::shared_ptr<::grpc::ServerCredentials>& raw() const {
    static std::shared_ptr<::grpc::ServerCredentials> null_ptr;
    return null_ptr;
  }
};

class InsecureServerCredentials : public ServerCredentials {
  Q_OBJECT

public:
  explicit InsecureServerCredentials(QObject* p = nullptr)
      : ServerCredentials(p) {}

  const std::shared_ptr<::grpc::ServerCredentials>& raw() const final {
    if (!raw_) {
      raw_ = ::grpc::InsecureServerCredentials();
    }
    return raw_;
  }

private:
  mutable std::shared_ptr<::grpc::ServerCredentials> raw_;
};

// TODO: Add SslServerCredentials class
}
}
#endif  // GRPC_QML_SERVER_CREDENTIALS_H
