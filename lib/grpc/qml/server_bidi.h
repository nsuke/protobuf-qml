#ifndef GRPC_QML_SERVER_BIDI_H
#define GRPC_QML_SERVER_BIDI_H

#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/server_method.h"

#include <grpc++/stream.h>
#include <grpc++/server_context.h>
#include <grpc++/impl/proto_utils.h>
#include <google/protobuf/message.h>
#include <queue>

namespace grpc {
namespace qml {

class GrpcService;
class ServerBidiCallData;
class ServerBidiMethod : public ::protobuf::qml::ServerReaderWriterMethod,
                         private CallDataStore<ServerBidiCallData> {
  Q_OBJECT

public:
  ServerBidiMethod(GrpcService* service,
                   int index,
                   ::grpc::ServerCompletionQueue* cq,
                   ::protobuf::qml::DescriptorWrapper* read,
                   ::protobuf::qml::DescriptorWrapper* write);

  // To be called from IO thread
  void onData(ServerBidiCallData* cdata, QVariant data);
  void onDataEnd(ServerBidiCallData* cdata);

  void startProcessing() final;
  bool respond(int tag, const QVariant& data) final;
  bool abort(int tag, int code, const QString& message) final;
  bool end(int tag) final;

private:
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  GrpcService* service_;
};

class ServerBidiCallData : public CallData {
public:
  ServerBidiCallData(ServerBidiMethod* method,
                     GrpcService* service,
                     int index,
                     ::grpc::ServerCompletionQueue* cq,
                     ::protobuf::qml::DescriptorWrapper* read,
                     ::protobuf::qml::DescriptorWrapper* write);

  void process(bool ok) final;
  void write(const QVariant& data);
  void writesDone();
  void abort(int code, const QString& message);
  int tag = 0;

private:
  enum class Status {
    INIT,
    REQUEST,
    READ,
    FROZEN,
    WRITE,
    DONE,
  };
  void handleQueuedMessages();
  void decrementRef(std::unique_lock<std::mutex>& lock, bool terminate = false);

  int ref_count_ = 1;
  Status status_ = Status::INIT;
  ::grpc::ServerContext context_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::unique_ptr<google::protobuf::Message> response_;
  ServerBidiMethod* method_;
  ::grpc::ServerAsyncReaderWriter<google::protobuf::Message,
                                  google::protobuf::Message> stream_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  int tag_;
  std::mutex mutex_;
  GrpcService* service_;
  std::queue<std::unique_ptr<google::protobuf::Message>> queue_;
};
}
}
#endif  // GRPC_QML_SERVER_BIDI_H
