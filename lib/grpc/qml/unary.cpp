#include "grpc/qml/unary.h"
#include <chrono>
#include <iostream>

namespace grpc {
namespace qml {

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

UnaryCallData::UnaryCallData(int tag,
                             UnaryMethod* method,
                             grpc::Channel* channel,
                             ::grpc::CompletionQueue* cq,
                             ::protobuf::qml::DescriptorWrapper* read,
                             std::unique_ptr<google::protobuf::Message> request,
                             int timeout)
    : cq_(cq),
      tag_(tag),
      method_(method),
      channel_(channel),
      read_(read),
      request_(std::move(request)),
      response_(read_->newMessage()) {
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  }
  reader_.reset(new grpc::ClientAsyncResponseReader<google::protobuf::Message>(
      channel_, cq_, method_->raw(), &context_, *request_));
  process(true);
}

UnaryCallData::~UnaryCallData() {}

void UnaryCallData::process(bool ok) {
  if (status_ == Status::READ) {
    status_ = Status::DONE;
    reader_->Finish(response_.get(), &grpc_status_, this);
  } else if (status_ == Status::DONE) {
    if (ok) {
      method_->data(tag_, response_);
    } else {
      std::cerr << grpc_status_.error_message() << std::endl;
      method_->error(tag_, grpc_status_.error_code(),
                     QString::fromStdString(grpc_status_.error_message()));
    }
    method_->closed(tag_);
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

UnaryMethod::UnaryMethod(const std::string& name,
                         ::protobuf::qml::DescriptorWrapper* read,
                         ::protobuf::qml::DescriptorWrapper* write,
                         std::shared_ptr<grpc::Channel> channel,
                         grpc::CompletionQueue* cq,
                         QObject* p)
    : ::protobuf::qml::UnaryMethod(p),
      name_(name),
      read_(read),
      cq_(cq),
      channel_(std::move(channel)),
      raw_(name.c_str(), grpc::RpcMethod::NORMAL_RPC, channel_) {}

UnaryMethod::~UnaryMethod() {}

bool UnaryMethod::write(int tag,
                        std::unique_ptr<google::protobuf::Message> request,
                        int timeout) {
  if (!request) {
    unknownError(tag, "No message object.");
    return false;
  }
  new UnaryCallData(tag, this, channel_.get(), cq_, read_, std::move(request),
                    timeout);
  return true;
}
}
}
