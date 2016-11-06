#ifndef GRPC_QML_UNARY_H
#define GRPC_QML_UNARY_H

#include "grpc/qml/server_calldata.h"
#include "grpc/qml/base.h"
#include "protobuf/qml/method.h"
#include <grpc++/support/async_unary_call.h>
#include <grpc++/impl/codegen/proto_utils.h>
#include <mutex>
#include <unordered_map>

namespace grpc {
namespace qml {

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
