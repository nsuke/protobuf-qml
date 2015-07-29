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
      response_(read_->newMessage()) {
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  }
  reader_.reset(new grpc::ClientAsyncResponseReader<google::protobuf::Message>(
      channel_, cq_, method_->raw(), &context_, *request_));
  process(true);
}

UnaryCallData::~UnaryCallData() {
}

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
                         std::shared_ptr<grpc::ChannelInterface> channel,
                         grpc::CompletionQueue* cq,
                         QObject* p)
    : ::protobuf::qml::UnaryMethod(p),
      name_(name),
      read_(read),
      cq_(cq),
      channel_(std::move(channel)),
      raw_(name.c_str(),
           grpc::RpcMethod::NORMAL_RPC,
           channel_->RegisterMethod(name.c_str())) {
}

UnaryMethod::~UnaryMethod() {
}

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
