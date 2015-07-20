#include "grpc/qml/server_unary.h"
#include "grpc/qml/server.h"
#include <grpc++/status_code_enum.h>

namespace grpc {
namespace qml {
ServerUnaryMethod::ServerUnaryMethod(GrpcService* service,
                                     int index,
                                     ::grpc::ServerCompletionQueue* cq,
                                     ::protobuf::qml::DescriptorWrapper* read,
                                     ::protobuf::qml::DescriptorWrapper* write)
    : read_(read), write_(write), cq_(cq), index_(index), service_(service) {
}

void ServerUnaryMethod::startProcessing() {
  new ServerUnaryCallData(this, service_, index_, cq_, read_, write_);
}

void ServerUnaryMethod::onRequest(ServerUnaryCallData* cdata) {
  auto tag = store(cdata);
  data(tag, cdata->data());
}

bool ServerUnaryMethod::respond(int tag, const QVariant& data) {
  auto cdata = remove(tag);
  if (!cdata) {
    qWarning() << "Unknown tag: " << tag;
    return false;
  }
  cdata->resume(data);
  return true;
}

bool ServerUnaryMethod::abort(int tag,
                              int error_code,
                              const QString& error_message) {
  auto cdata = remove(tag);
  if (!cdata) {
    qWarning() << "Unknown tag: " << tag;
    return false;
  }
  cdata->abort(error_code, error_message);
  return true;
}

ServerUnaryCallData::ServerUnaryCallData(
    ServerUnaryMethod* method,
    GrpcService* service,
    int index,
    ::grpc::ServerCompletionQueue* cq,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write)
    : read_(read),
      write_(write),
      method_(method),
      writer_(&context_),
      cq_(cq),
      index_(index),
      service_(service) {
  process(true);
}

void ServerUnaryCallData::process(bool ok) {
  if (status_ == Status::INIT) {
    request_.reset(read_->newMessage());
    service_->raw()->RequestAsyncUnary(index_, &context_, request_.get(),
                                       &writer_, cq_, cq_, this);
    status_ = Status::READ;
  } else if (status_ == Status::READ) {
    if (!ok) {
      // init called after shutdown ?
      delete this;
      return;
    }
    status_ = Status::FROZEN;
    method_->onRequest(this);
  } else if (status_ == Status::DONE) {
    new ServerUnaryCallData(method_, service_, index_, cq_, read_, write_);
    if (!ok) {
      // notify
    }
    method_->closed(tag_);
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

void ServerUnaryCallData::resume(const QVariant& data) {
  if (status_ != Status::FROZEN) {
    qWarning() << "Resume called for non-frozen call data.";
    Q_ASSERT(false);
    return;
  }
  response_.reset(write_->dataToMessage(data));
  if (!response_) {
    // TODO: how to abort from here ?
  }
  status_ = Status::DONE;
  writer_.Finish(*response_, grpc::Status::OK, this);
}

void ServerUnaryCallData::abort(int error_code, const QString& error_message) {
  if (status_ != Status::FROZEN) {
    qWarning() << "Resume called for non-frozen call data.";
    Q_ASSERT(false);
    return;
  }
  status_ = Status::DONE;
  grpc::Status error_status(static_cast<grpc::StatusCode>(error_code),
                            error_message.toStdString());
  writer_.FinishWithError(error_status, this);
}
}
}
