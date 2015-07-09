#include "grpc/qml/unary.h"
#include <iostream>
#include <chrono>

namespace grpc {
namespace qml {

UnaryCallData::UnaryCallData(int tag,
                             UnaryMethod* method,
                             grpc::ChannelInterface* channel,
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
      response_(read_->newMessage()),
      reader_(channel_, cq_, method_->raw(), &context_, *request_) {
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  }
  process(true);
}

UnaryCallData::~UnaryCallData() {
}

void UnaryCallData::process(bool ok) {
  if (status_ == Status::INIT) {
    status_ = Status::DONE;
    reader_.Finish(response_.get(), &grpc_status_, this);
  } else if (status_ == Status::DONE) {
    if (ok) {
      // TODO: check status
      auto data = read_->dataFromMessage(*response_);
      method_->data(tag_, data);
    } else {
      std::cerr << grpc_status_.error_message() << std::endl;
      method_->error(tag_,
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
                         std::shared_ptr<grpc::ChannelInterface> channel,
                         grpc::CompletionQueue* cq,
                         QObject* p)
    : ::protobuf::qml::UnaryMethod(p),
      name_(name),
      read_(read),
      write_(write),
      channel_(std::move(channel)),
      cq_(cq),
      raw_(name.c_str(),
           grpc::RpcMethod::NORMAL_RPC,
           channel_->RegisterMethod(name.c_str())) {
}

UnaryMethod::~UnaryMethod() {
}

bool UnaryMethod::write(int tag, const QVariant& data, int timeout) {
  std::unique_ptr<google::protobuf::Message> request(
      write_->dataToMessage(data));
  if (!request) {
    error(tag, "Failed to convert to message object.");
    return false;
  }
  new UnaryCallData(tag, this, channel_.get(), cq_, read_, std::move(request),
                    timeout);
  return true;
}
}
}
