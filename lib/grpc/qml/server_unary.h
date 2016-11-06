#ifndef GRPC_QML_SERVER_UNARY_H
#define GRPC_QML_SERVER_UNARY_H

#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/server_method.h"

#include <grpc++/support/async_unary_call.h>
#include <grpc++/server_context.h>
#include <grpc++/impl/codegen/proto_utils.h>
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
  bool respond(int tag, std::unique_ptr<google::protobuf::Message>) final;
  bool abort(int tag, int error_code, const QString& error_message) final;

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
  void resume(std::unique_ptr<google::protobuf::Message>);
  void abort(int error_code, const QString& error_message);
  const std::shared_ptr<google::protobuf::Message>& data() const {
    return request_;
  }

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
  std::shared_ptr<google::protobuf::Message> request_;
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
