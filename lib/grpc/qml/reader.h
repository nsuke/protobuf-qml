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
  ~ReaderCallData();

  void process(bool ok) final;
  void resume(std::unique_ptr<google::protobuf::Message> data);

  void write();

private:
  enum class Status {
    WRITE,
    READ,
    DONE,
  };

  Status status_ = Status::WRITE;
  grpc::ClientContext context_;
  grpc::CompletionQueue* cq_;
  grpc::ChannelInterface* channel_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ReaderMethod* method_;
  int tag_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::shared_ptr<google::protobuf::Message> response_;
  grpc::Status grpc_status_;
  std::unique_ptr<grpc::ClientAsyncReader<google::protobuf::Message>> reader_;
  std::mutex mutex_;
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
        cq_(cq),
        channel_(std::move(channel)),
        raw_(name.c_str(),
             grpc::RpcMethod::SERVER_STREAMING,
             channel_->RegisterMethod(name.c_str())) {}

  bool write(int tag,
             std::unique_ptr<google::protobuf::Message> data,
             int timeout) final;

  const grpc::RpcMethod& raw() const { return raw_; }

private:
  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::ChannelInterface> channel_;
  grpc::RpcMethod raw_;
};
}
}
#endif  // GRPC_QML_READER_H
