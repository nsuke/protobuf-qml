#ifndef GRPC_QML_SERVER_H
#define GRPC_QML_SERVER_H

#include "grpc/qml/server_credentials.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/server_method.h"

#include <grpc++/stream.h>
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
#include <unordered_map>

namespace grpc {
namespace qml {

class CallData {
public:
  virtual ~CallData() {}
  virtual void process(bool ok) = 0;

protected:
  CallData() {}

private:
  CallData(const CallData&) = delete;
  CallData& operator=(const CallData&) = delete;
};

class GrpcService;

class ServerUnaryCallData;

class ServerUnaryMethod : public ::protobuf::qml::ServerUnaryMethod {
  Q_OBJECT

public:
  ServerUnaryMethod(GrpcService* service,
                    int index,
                    ::grpc::ServerCompletionQueue* cq,
                    ::protobuf::qml::DescriptorWrapper* read,
                    ::protobuf::qml::DescriptorWrapper* write);

  // To be called from IO thread
  void onRequest(ServerUnaryCallData* cdata);

  void startProcessing() final;
  bool respond(int tag, const QVariant& data) final;

private:
  int store(ServerUnaryCallData* cdata) {
    auto res = cdata_.insert(std::make_pair(++tag_, cdata));
    if (res.second) {
      return res.first->first;
    } else {
      return store(cdata);
    }
  }
  ServerUnaryCallData* remove(int tag) {
    // TODO: erase
    auto it = cdata_.find(tag);
    if (it == cdata_.end()) {
      return nullptr;
    }
    return it->second;
  }

  int tag_ = 1;
  std::unordered_map<int, ServerUnaryCallData*> cdata_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  GrpcService* service_;
};

class ServerUnaryCallData : public CallData {
public:
  ServerUnaryCallData(
      ServerUnaryMethod*
      method, GrpcService* service,
                      int index,
                      ::grpc::ServerCompletionQueue* cq,
                      ::protobuf::qml::DescriptorWrapper* read,
                      ::protobuf::qml::DescriptorWrapper* write);

  void process(bool ok) final;
  void resume(const QVariant& data);
  const QVariant& data() const { return data_; }

private:
  enum class Status {
    INIT,
    READ,
    FROZEN,
    WRITE,
    DONE,
  };

  Status status_ = Status::INIT;
  ::grpc::ServerContext context_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::unique_ptr<google::protobuf::Message> response_;
  QVariant data_;
  ServerUnaryMethod* method_;
  ::grpc::ServerAsyncWriter<google::protobuf::Message> writer_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  int tag_;
  GrpcService* service_;
};

class RawGrpcService : public ::grpc::AsynchronousService {
public:
  RawGrpcService(const char** names, int count)
      : ::grpc::AsynchronousService(names, count) {}

private:
  friend class ServerUnaryCallData;
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
    qDebug() << "SHUTDOWN";
    if (cq_) {
    qDebug() << "SHUTDOWN";
      cq_->Shutdown();
    }
    // if (thread_ && thread_->joinable()) {
    // qDebug() << "JOIN";
    //    thread_->join();
    // }
    //   //thread_->detach();
    //  thread_.reset();
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
