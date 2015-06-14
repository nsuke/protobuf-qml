#ifndef GRPC_GRML_WRITER_H
#define GRPC_GRML_WRITER_H

#include "protobuf/qml/method.h"
#include "grpc/qml/base.h"
#include <mutex>
#include <unordered_map>

namespace grpc {
namespace qml {

class WriterMethod;

class WriterCall {
public:
  WriterCall(int tag,
             grpc::ChannelInterface* channel,
             grpc::CompletionQueue* cq,
             WriterMethod* method,
             ::protobuf::qml::DescriptorWrapper* read_desc)
      : tag_(tag),
        channel_(channel),
        cq_(cq),
        method_(method),
        read_desc_(read_desc),
        response_(read_desc->newMessage()) {}

  bool write(std::unique_ptr<google::protobuf::Message> request);

  bool writesDone(int timeout);

  void finish();

  int tag() const { return tag_; }
  WriterMethod* method() { return method_; }

  ::protobuf::qml::DescriptorWrapper* read_descriptor() { return read_desc_; }

  google::protobuf::Message* response() { return response_.get(); }

private:
  void ensureInit();

  int tag_;
  grpc::ChannelInterface* channel_;
  grpc::CompletionQueue* cq_;
  WriterMethod* method_;
  grpc::ClientContext context_;
  ::protobuf::qml::DescriptorWrapper* read_desc_;
  std::unique_ptr<google::protobuf::Message> response_;
  std::unique_ptr<grpc::ClientAsyncWriter<google::protobuf::Message>> writer_;
};

class WriterMethod : public ::protobuf::qml::WriterMethod {
  Q_OBJECT

public:
  WriterMethod(const std::string& name,
               ::protobuf::qml::DescriptorWrapper* read,
               ::protobuf::qml::DescriptorWrapper* write,
               std::shared_ptr<grpc::ChannelInterface> channel,
               grpc::CompletionQueue* cq,
               QObject* p = nullptr)
      : ::protobuf::qml::WriterMethod(p),
        name_(name),
        read_(read),
        write_(write),
        channel_(std::move(channel)),
        cq_(cq),
        raw_(name.c_str(),
             grpc::RpcMethod::CLIENT_STREAMING,
             channel_->RegisterMethod(name.c_str())) {}

  bool write(int tag, const QVariant& data, int timeout) final;

  bool writesDone(int tag, int timeout) final;

  void deleteCall(int tag) {  // TODO:
  }

  const grpc::RpcMethod& raw() const { return raw_; }

private:
  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::ChannelInterface> channel_;
  grpc::RpcMethod raw_;
  std::mutex calls_mutex_;
  std::unordered_map<int, WriterCall> calls_;
};
}
}
#endif  // GRPC_GRML_WRITER_H
