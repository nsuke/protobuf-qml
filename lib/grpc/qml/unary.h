#ifndef GRPC_QML_UNARY_H
#define GRPC_QML_UNARY_H

#include "grpc/qml/server_calldata.h"
#include "grpc/qml/base.h"
#include "protobuf/qml/method.h"
#include <grpc++/support/async_unary_call.h>
#include <grpc++/impl/proto_utils.h>
#include <mutex>
#include <unordered_map>

namespace grpc {
namespace qml {

class UnaryMethod;

class UnaryCallData : public CallData {
public:
  UnaryCallData(int tag,
                UnaryMethod* method,
                grpc::Channel* channel,
                ::grpc::CompletionQueue* cq,
                ::protobuf::qml::DescriptorWrapper* read,
                std::unique_ptr<google::protobuf::Message> request,
                int timeout);

  ~UnaryCallData();

  void process(bool ok) final;

private:
  enum class Status {
    READ,
    DONE,
  };

  Status status_ = Status::READ;
  ::grpc::CompletionQueue* cq_;
  ::grpc::ClientContext context_;
  int tag_;
  UnaryMethod* method_;
  grpc::Channel* channel_;
  ::protobuf::qml::DescriptorWrapper* read_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::shared_ptr<google::protobuf::Message> response_;
  grpc::Status grpc_status_;
  std::unique_ptr<grpc::ClientAsyncResponseReader<google::protobuf::Message>>
      reader_;
  std::mutex mutex_;
};

class UnaryMethod : public ::protobuf::qml::UnaryMethod {
  Q_OBJECT

public:
  UnaryMethod(const std::string& name,
              ::protobuf::qml::DescriptorWrapper* read,
              ::protobuf::qml::DescriptorWrapper* write,
              std::shared_ptr<grpc::Channel> channel,
              grpc::CompletionQueue* cq,
              QObject* p = nullptr);

  ~UnaryMethod();

  bool write(int tag,
             std::unique_ptr<google::protobuf::Message> msg,
             int timeout) final;

  grpc::RpcMethod& raw() { return raw_; }

  ::protobuf::qml::DescriptorWrapper* readDescriptor() { return read_; }

private:
  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::Channel> channel_;
  grpc::RpcMethod raw_;
  std::mutex calls_mutex_;
};
}
}

#endif  // GRPC_QML_UNARY_H
