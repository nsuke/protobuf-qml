#ifndef GRPC_QML_SERVER_UNARY_H
#define GRPC_QML_SERVER_UNARY_H

#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/server_method.h"

#include <grpc++/async_unary_call.h>
#include <grpc++/server_context.h>
#include <grpc++/impl/proto_utils.h>
#include <google/protobuf/message.h>

namespace grpc {
namespace qml {

class GrpcService;
class ServerUnaryCallData;

class ServerUnaryMethod : public ::protobuf::qml::ServerUnaryMethod,
                          private CallDataStore<ServerUnaryCallData> {
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
  ::grpc::ServerAsyncResponseWriter<google::protobuf::Message> writer_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  int tag_;
  GrpcService* service_;
};
}
}
#endif  // GRPC_QML_SERVER_UNARY_H
