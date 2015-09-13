#ifndef GRPC_QML_SERVER_H
#define GRPC_QML_SERVER_H

#include "grpc/qml/server_credentials.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/server_method.h"

#include <grpc++/support/async_stream.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/impl/service_type.h>
#include <grpc++/impl/proto_utils.h>
#include <google/protobuf/message.h>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <deque>
#include <memory>
#include <thread>

namespace grpc {
namespace qml {

class RawGrpcService : public ::grpc::AsynchronousService {
public:
  RawGrpcService(const char** names, int count)
      : ::grpc::AsynchronousService(names, count) {}

private:
  friend class ServerUnaryCallData;
  friend class ServerReaderCallData;
  friend class ServerWriterCallData;
  friend class ServerBidiCallData;
};

class GrpcService {
public:
  explicit GrpcService(::protobuf::qml::RpcService* service)
      : service_(service) {
    auto& methods = service->methods();
    raw_method_names_.reserve(methods.size());
    raw_method_names_cstr_.reserve(methods.size());
    for (auto& method : methods) {
      raw_method_names_.push_back(method->method_name().toStdString());
      raw_method_names_cstr_.push_back(raw_method_names_.back().c_str());
    }
    raw_.reset(new RawGrpcService(raw_method_names_cstr_.data(),
                                  raw_method_names_cstr_.size()));
  }

  RawGrpcService* raw() { return raw_.get(); }

  ::protobuf::qml::RpcService* service() { return service_; }

private:
  ::protobuf::qml::RpcService* service_;
  std::vector<std::string> raw_method_names_;
  std::vector<const char*> raw_method_names_cstr_;
  std::unique_ptr<RawGrpcService> raw_;
};

class GrpcServer : public ::protobuf::qml::RpcServer {
  Q_OBJECT
  Q_PROPERTY(QString address READ address WRITE set_address NOTIFY
                 addressChanged)
  Q_PROPERTY(grpc::qml::ServerCredentials* credentials READ credentials WRITE
                 set_credentials NOTIFY credentialsChanged)

signals:
  void addressChanged();
  void credentialsChanged();

public:
  ~GrpcServer();

  const QString& address() const { return address_; }
  void set_address(const QString& address) {
    if (address_ != address) {
      address_ = address;
      addressChanged();
    }
  }

  ServerCredentials* credentials() const { return credentials_; }
  void set_credentials(ServerCredentials* credentials) {
    if (credentials_ != credentials) {
      credentials_ = credentials;
      credentialsChanged();
    }
  }

  bool registerService(::protobuf::qml::RpcService*) final;

  Q_INVOKABLE void shutdown() {
    if (server_) {
      server_.reset();
      cq_->Shutdown();
    }
  }

protected:
  bool doStart() final;

private:
  void handle();

  QString address_;
  ServerCredentials* credentials_ = nullptr;
  std::unique_ptr<::grpc::ServerCompletionQueue> cq_;
  std::deque<GrpcService> services_;
  std::unique_ptr<::grpc::Server> server_;
  std::unique_ptr<std::thread> thread_;
};
}
}

#endif  // GRPC_QML_SERVER_H
