#ifndef GRPC_QML_READER_H
#define GRPC_QML_READER_H

#include "grpc/qml/base.h"
#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/method.h"

#include <grpc++/impl/proto_utils.h>

namespace grpc {
namespace qml {

class ReaderMethod;

class ReaderCallData : public CallData {
public:
  ReaderCallData(int tag,
                 grpc::ChannelInterface* channel,
                 ::grpc::CompletionQueue* cq,
                 ReaderMethod* method,
                 ::protobuf::qml::DescriptorWrapper* read,
                 std::unique_ptr<google::protobuf::Message> request,
                 int timeout);

  void process(bool ok) final;
  void resume(const QVariant& data);

  void write();

private:
  enum class Status {
    WRITE,
    READ,
    DONE,
  };

  Status status_ = Status::WRITE;
  grpc::ClientContext context_;
  ::protobuf::qml::DescriptorWrapper* read_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::unique_ptr<google::protobuf::Message> response_;
  ReaderMethod* method_;
  grpc::CompletionQueue* cq_;
  int tag_;
  grpc::ChannelInterface* channel_;
  grpc::Status grpc_status_;
  std::unique_ptr<grpc::ClientAsyncReader<google::protobuf::Message>> reader_;
};

class ReaderMethod : public ::protobuf::qml::ReaderMethod {
  Q_OBJECT

public:
  ReaderMethod(const std::string& name,
               ::protobuf::qml::DescriptorWrapper* read,
               ::protobuf::qml::DescriptorWrapper* write,
               std::shared_ptr<grpc::ChannelInterface> channel,
               grpc::CompletionQueue* cq,
               QObject* p = nullptr)
      : ::protobuf::qml::ReaderMethod(p),
        name_(name),
        read_(read),
        write_(write),
        channel_(std::move(channel)),
        cq_(cq),
        raw_(name.c_str(),
             grpc::RpcMethod::SERVER_STREAMING,
             channel_->RegisterMethod(name.c_str())) {}

  bool write(int tag, const QVariant& data, int timeout);

  const grpc::RpcMethod& raw() const { return raw_; }

private:
  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::ChannelInterface> channel_;
  grpc::RpcMethod raw_;
};
}
}
#endif  // GRPC_QML_READER_H
