#include "grpc/qml/server_unary.h"
#include "grpc/qml/server.h"

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
    qWarning() << "";
    return false;
  }
  cdata->resume(data);
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
    data_ = read_->dataFromMessage(*request_);
    status_ = Status::FROZEN;
    method_->onRequest(this);
  } else if (status_ == Status::WRITE) {
    if (!ok) {
      // notify
    }
    status_ = Status::DONE;
    writer_.Finish(grpc::Status::OK, this);
  } else if (status_ == Status::DONE) {
    new ServerUnaryCallData(method_, service_, index_, cq_, read_, write_);
    if (!ok) {
      // notify
    }
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

void ServerUnaryCallData::resume(const QVariant& data) {
  if (status_ != Status::FROZEN) {
    qWarning() << "Resume called for non-frozen call data.";
    return;
  }
  response_.reset(write_->dataToMessage(data));
  if (!response_) {
    // TODO: how to abort from here ?
  }
  status_ = Status::WRITE;
  writer_.Write(*response_, this);
}
}
}
