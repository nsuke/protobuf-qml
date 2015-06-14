#include "grpc/qml/unary.h"
#include <grpc++/async_unary_call.h>
#include <iostream>
#include <chrono>

namespace grpc {
namespace qml {

class UnaryOp : public CallOp {
public:
  UnaryOp(UnaryMethod* method,
          UnaryCall* call,
          int tag,
          ::protobuf::qml::DescriptorWrapper* read_desc)
      : method_(method),
        call_(call),
        tag_(tag),
        read_desc_(read_desc),
        message_(read_desc->newMessage()) {}

  ~UnaryOp() {
    qDebug() << __PRETTY_FUNCTION__;
    method_->deleteCall(tag_);
  }

  void onEvent(bool*) final {
    qDebug() << __PRETTY_FUNCTION__;
    // TODO: check status
    auto data = read_desc_->dataFromMessage(*message_);
    method_->data(tag_, data);
  }

  void onError(bool timeout) final {
    qDebug() << __PRETTY_FUNCTION__;

    // TODO: Deliver error message to the caller.
    std::cerr << status.details() << std::endl;

    auto msg = timeout ? "Timeout." : "Unknown error";
    method_->error(tag_, msg);
  }

  google::protobuf::Message* response() { return message_.get(); }

private:
  UnaryMethod* method_;
  UnaryCall* call_;
  int tag_;
  ::protobuf::qml::DescriptorWrapper* read_desc_;
  std::unique_ptr<google::protobuf::Message> message_;
};

UnaryCall::UnaryCall(int tag,
                     UnaryMethod* method,
                     grpc::ChannelInterface* channel,
                     grpc::CompletionQueue* cq,
                     std::unique_ptr<google::protobuf::Message> request,
                     int timeout)
    : tag_(tag),
      channel_(channel),
      cq_(cq),
      method_(method),
      request_(std::move(request)) {
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  }
}

UnaryCall::~UnaryCall() {
}

void UnaryCall::write() {
  reader_.reset(new grpc::ClientAsyncResponseReader<google::protobuf::Message>(
      channel_, cq_, method_->raw(), &context_, *request_));
  std::unique_ptr<UnaryOp> op(
      new UnaryOp(method_, this, tag_, method_->readDescriptor()));
  auto rsp = op->response();
  auto st = &op->status;
  reader_->Finish(rsp, st, op.release());
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
  UnaryCall* call = nullptr;
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    auto it = calls_.find(tag);
    if (it != calls_.end()) {
      // Since tag is used by another call, we should not emit the error to
      // tag.
      // error(tag, "Tag already in use");
      qWarning() << "Tag already in use: " << tag;
      return false;
    }
    auto res =
        calls_.emplace(std::piecewise_construct, std::forward_as_tuple(tag),
                       std::forward_as_tuple(tag, this, channel_.get(), cq_,
                                             std::move(request), timeout));
    if (!res.second) {
      error(tag, "Failed to create call object");
      return false;
    }
    call = &res.first->second;
  }
  Q_ASSERT(call);
  call->write();
  return true;
}

void UnaryMethod::deleteCall(int tag) {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto it = calls_.find(tag);
  if (it == calls_.end()) {
    qWarning() << "Invalid call tag: " << tag;
    return;
  }
  calls_.erase(it);
  closed(tag);
}
}
}
