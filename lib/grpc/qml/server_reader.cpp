#include "grpc/qml/server_reader.h"
#include "grpc/qml/server.h"

namespace grpc {
namespace qml {

ServerReaderMethod::ServerReaderMethod(
    GrpcService* service,
    int index,
    ::grpc::ServerCompletionQueue* cq,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write)
    : read_(read), write_(write), cq_(cq), index_(index), service_(service) {
}

void ServerReaderMethod::startProcessing() {
  new ServerReaderCallData(this, service_, index_, cq_, read_, write_);
}

void ServerReaderMethod::onData(
    ServerReaderCallData* cdata
    //, std::unique_ptr<google::protobuf::Message> data
    ) {
  Q_ASSERT(cdata);
  if (!cdata->tag) {
    cdata->tag = store(cdata);
  }
  data(cdata->tag, cdata->data());
}

void ServerReaderMethod::onDataEnd(ServerReaderCallData* cdata) {
  Q_ASSERT(cdata);
  if (cdata->tag) {
    dataEnd(cdata->tag);
  }
}

bool ServerReaderMethod::respond(int tag, const QVariant& data) {
  auto cdata = remove(tag);
  if (!cdata) {
    qWarning() << "Unknown tag to respond: " << tag;
    return false;
  }
  cdata->resume(data);
  return true;
}

bool ServerReaderMethod::abort(int tag, int code, const QString& message) {
  auto cdata = remove(tag);
  if (!cdata) {
    qWarning() << "Unknown tag to abort: " << tag;
    return false;
  }
  cdata->abort(code, message);
  return true;
}

ServerReaderCallData::ServerReaderCallData(
    ServerReaderMethod* method,
    GrpcService* service,
    int index,
    ::grpc::ServerCompletionQueue* cq,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write)
    : read_(read),
      write_(write),
      method_(method),
      reader_(&context_),
      cq_(cq),
      index_(index),
      service_(service) {
  process(true);
}

void ServerReaderCallData::process(bool ok) {
  if (status_ == Status::INIT) {
    request_.reset(read_->newMessage());
    service_->raw()->RequestClientStreaming(index_, &context_, &reader_, cq_,
                                            cq_, this);
    status_ = Status::FIRST_READ;
  } else if (status_ == Status::FIRST_READ && ok) {
    reader_.Read(request_.get(), this);
    status_ = Status::READ;
  } else if (status_ == Status::FIRST_READ) {
    // init called after shutdown ?
    // TODO: handle shutdown more explicitly
    delete this;
  } else if (status_ == Status::READ && ok) {
    data_ = read_->dataFromMessage(*request_);
    method_->onData(this);
    reader_.Read(request_.get(), this);
  } else if (status_ == Status::READ) {
    // client streaming completed
    status_ = Status::FROZEN;
    method_->onDataEnd(this);
  } else if (status_ == Status::DONE) {
    new ServerReaderCallData(method_, service_, index_, cq_, read_, write_);
    if (!ok) {
      // notify
    }
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

void ServerReaderCallData::resume(const QVariant& data) {
  if (status_ != Status::FROZEN) {
    qWarning() << "Resume called for non-frozen call data.";
    return;
  }
  response_.reset(write_->dataToMessage(data));
  if (!response_) {
    // TODO: how to abort from here ?
  }
  status_ = Status::DONE;
  reader_.Finish(*response_, grpc::Status::OK, this);
}

void ServerReaderCallData::abort(int code, const QString& message) {
  if (status_ != Status::FROZEN) {
    qWarning() << "Abort called for non-frozen call data.";
    return;
  }
  status_ = Status::DONE;
  grpc::Status grpc_status(static_cast<grpc::StatusCode>(code),
                           message.toStdString());
  reader_.FinishWithError(grpc_status, this);
}
}
}
