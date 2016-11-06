#ifndef GRPC_GRML_WRITER_H
#define GRPC_GRML_WRITER_H

#include "grpc/qml/base.h"
#include "protobuf/qml/method.h"

#include <mutex>
#include <unordered_map>

namespace grpc {
namespace qml {

class WriterCallData;

class WriterMethod final : public ::protobuf::qml::WriterMethod {
  Q_OBJECT

public:
  WriterMethod(const std::string& name,
               ::protobuf::qml::DescriptorWrapper* read,
               ::protobuf::qml::DescriptorWrapper* write,
               std::shared_ptr<grpc::Channel> channel,
               grpc::CompletionQueue* cq,
               QObject* p = nullptr);

  bool write(int tag, std::unique_ptr<google::protobuf::Message> data) override;

  bool writesDone(int tag) override;

  void deleteCall(int tag);

  int timeout(int tag) const override;
  void set_timeout(int tag, int milliseconds) final;

  const grpc::RpcMethod& raw() const { return raw_; }

private:
  WriterCallData* ensureCallData(int tag);

  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::Channel> channel_;
  grpc::RpcMethod raw_;
  mutable std::mutex calls_mutex_;
  std::unordered_map<int, WriterCallData*> calls_;
};
}
}
#endif  // GRPC_GRML_WRITER_H
