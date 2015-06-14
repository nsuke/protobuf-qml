#ifndef GRPC_QML_UNARY_H
#define GRPC_QML_UNARY_H

#include "protobuf/qml/method.h"
#include "grpc/qml/base.h"
#include <mutex>
#include <unordered_map>

namespace grpc {
namespace qml {

class UnaryMethod;

class UnaryCall {
public:
  UnaryCall(int tag,
            UnaryMethod* method,
            grpc::ChannelInterface* channel,
            grpc::CompletionQueue* cq,
            std::unique_ptr<google::protobuf::Message> request,
            int timeout);
  ~UnaryCall();

  void write();

private:
  int tag_;
  grpc::ChannelInterface* channel_;
  grpc::CompletionQueue* cq_;
  UnaryMethod* method_;
  grpc::ClientContext context_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::unique_ptr<grpc::ClientAsyncResponseReader<google::protobuf::Message>>
      reader_;
};

class UnaryMethod : public ::protobuf::qml::UnaryMethod {
  Q_OBJECT

public:
  UnaryMethod(const std::string& name,
              ::protobuf::qml::DescriptorWrapper* read,
              ::protobuf::qml::DescriptorWrapper* write,
              std::shared_ptr<grpc::ChannelInterface> channel,
              grpc::CompletionQueue* cq,
              QObject* p = nullptr);

  ~UnaryMethod();

  bool write(int tag, const QVariant& data, int timeout) final;

  void deleteCall(int tag);

  grpc::RpcMethod& raw() { return raw_; }

  ::protobuf::qml::DescriptorWrapper* readDescriptor() { return read_; }

  ::protobuf::qml::DescriptorWrapper* writeDescriptor() { return write_; }

private:
  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::ChannelInterface> channel_;
  grpc::RpcMethod raw_;
  std::mutex calls_mutex_;
  std::unordered_map<int, UnaryCall> calls_;
};
}
}

#endif  // GRPC_QML_UNARY_H
