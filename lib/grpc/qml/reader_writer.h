#ifndef GRPC_QML_READER_WRITER_H
#define GRPC_QML_READER_WRITER_H

#include "grpc/qml/base.h"
#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/method.h"

#include <grpc++/impl/proto_utils.h>
#include <queue>

namespace grpc {
namespace qml {

class ReaderWriterMethod;

class ReaderWriterCallData : public CallData {
public:
  ReaderWriterCallData(int tag,
                       grpc::ChannelInterface* channel,
                       ::grpc::CompletionQueue* cq,
                       ReaderWriterMethod* method,
                       ::protobuf::qml::DescriptorWrapper* read);

  void process(bool ok) final;
  void resume(const QVariant& data);

  bool write(std::unique_ptr<google::protobuf::Message> request);
  bool writesDone();
  int timeout() const { return timeout_; }
  void set_timeout(int timeout);

private:
  enum class Status {
    INIT,
    FROZEN,
    WRITE,
    READ,
    WRITES_DONE,
    FINISH,
  };
  void handleQueuedRequests();

  std::mutex mutex_;
  Status status_ = Status::INIT;
  bool read_done_ = false;
  bool write_done_ = false;
  bool write_done_queued_ = false;
  std::queue<std::unique_ptr<google::protobuf::Message>> requests_;
  int timeout_ = -1;
  grpc::ClientContext context_;
  ::protobuf::qml::DescriptorWrapper* read_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::unique_ptr<google::protobuf::Message> response_;
  ReaderWriterMethod* method_;
  grpc::CompletionQueue* cq_;
  int tag_;
  grpc::ChannelInterface* channel_;
  grpc::Status grpc_status_;
  grpc::ClientAsyncReaderWriter<google::protobuf::Message,
                                google::protobuf::Message> stream_;
};

class ReaderWriterMethod : public ::protobuf::qml::ReaderWriterMethod {
  Q_OBJECT

public:
  ReaderWriterMethod(const std::string& name,
                     ::protobuf::qml::DescriptorWrapper* read,
                     ::protobuf::qml::DescriptorWrapper* write,
                     std::shared_ptr<grpc::ChannelInterface> channel,
                     grpc::CompletionQueue* cq,
                     QObject* p = nullptr)
      : ::protobuf::qml::ReaderWriterMethod(p),
        name_(name),
        read_(read),
        write_(write),
        channel_(std::move(channel)),
        cq_(cq),
        raw_(name.c_str(),
             grpc::RpcMethod::BIDI_STREAMING,
             channel_->RegisterMethod(name.c_str())) {}

  bool call(int tag) final;
  bool write(int tag, const QVariant& data) final;
  bool writesDone(int tag) final;
  void deleteCall(int tag);
  int timeout(int tag) const final;
  void set_timeout(int tag, int milliseconds) final;

  const grpc::RpcMethod& raw() const { return raw_; }

private:
  ReaderWriterCallData* ensureCallData(int tag);

  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::ChannelInterface> channel_;
  grpc::RpcMethod raw_;
  mutable std::mutex calls_mutex_;
  std::unordered_map<int, ReaderWriterCallData*> calls_;
};
}
}

#endif  // GRPC_QML_READER_WRITER_H
