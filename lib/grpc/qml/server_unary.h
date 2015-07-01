#ifndef GRPC_QML_SERVER_UNARY_H
#define GRPC_QML_SERVER_UNARY_H

#include "grpc/qml/server_calldata.h"
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

#include <unordered_map>

namespace grpc {
namespace qml {

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
  ServerUnaryCallData(ServerUnaryMethod* method,
                      GrpcService* service,
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
}
}
#endif  // GRPC_QML_SERVER_UNARY_H
