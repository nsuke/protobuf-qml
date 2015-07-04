#ifndef GRPC_QML_SERVER_WRITER_H
#define GRPC_QML_SERVER_WRITER_H

#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/server_method.h"

#include <grpc++/stream.h>
#include <grpc++/server_context.h>
#include <grpc++/impl/proto_utils.h>
#include <google/protobuf/message.h>
#include <mutex>
#include <queue>

namespace grpc {
namespace qml {

class GrpcService;
class ServerWriterCallData;

class ServerWriterMethod : public ::protobuf::qml::ServerWriterMethod,
                           private CallDataStore<ServerWriterCallData> {
  Q_OBJECT

public:
  ServerWriterMethod(GrpcService* service,
                     int index,
                     ::grpc::ServerCompletionQueue* cq,
                     ::protobuf::qml::DescriptorWrapper* read,
                     ::protobuf::qml::DescriptorWrapper* write);

  // To be called from IO thread
  void onRequest(ServerWriterCallData* cdata, const QVariant& data);

  void startProcessing() final;
  bool respond(int tag, const QVariant& data) final;
  bool end(int tag) final;

private:
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  GrpcService* service_;
};

class ServerWriterCallData : public CallData {
public:
  ServerWriterCallData(ServerWriterMethod* method,
                       GrpcService* service,
                       int index,
                       ::grpc::ServerCompletionQueue* cq,
                       ::protobuf::qml::DescriptorWrapper* read,
                       ::protobuf::qml::DescriptorWrapper* write);

  void process(bool ok) final;
  void write(const QVariant& data);
  void end();

private:
  enum class Status {
    INIT,
    READ,
    FROZEN,
    WRITE,
    DONE,
  };

  void enqueueData(const QVariant& data);
  void processQueuedData();

  std::mutex mutex_;
  Status status_ = Status::INIT;
  ::grpc::ServerContext context_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::unique_ptr<google::protobuf::Message> response_;
  std::queue<std::unique_ptr<google::protobuf::Message>> queue_;
  ServerWriterMethod* method_;
  ::grpc::ServerAsyncWriter<google::protobuf::Message> writer_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  int tag_;
  GrpcService* service_;
};
}
}

#endif  // GRPC_QML_SERVER_WRITER_H
