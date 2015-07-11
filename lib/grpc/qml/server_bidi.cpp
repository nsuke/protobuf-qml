#include "grpc/qml/server_bidi.h"
#include "grpc/qml/server.h"

namespace grpc {
namespace qml {

ServerBidiMethod::ServerBidiMethod(GrpcService* service,
                                   int index,
                                   ::grpc::ServerCompletionQueue* cq,
                                   ::protobuf::qml::DescriptorWrapper* read,
                                   ::protobuf::qml::DescriptorWrapper* write)
    : read_(read), write_(write), cq_(cq), index_(index), service_(service) {
}

void ServerBidiMethod::startProcessing() {
  new ServerBidiCallData(this, service_, index_, cq_, read_, write_);
}

void ServerBidiMethod::onData(ServerBidiCallData* cdata, QVariant v) {
  Q_ASSERT(cdata);
  if (!cdata->tag) {
    cdata->tag = store(cdata);
  }
  data(cdata->tag, v);
}

void ServerBidiMethod::onDataEnd(ServerBidiCallData* cdata) {
  Q_ASSERT(cdata);
  if (cdata->tag) {
    dataEnd(cdata->tag);
  }
}

bool ServerBidiMethod::respond(int tag, const QVariant& data) {
  auto lk = lock();
  auto cdata = getUnsafe(tag);
  if (!cdata) {
    qWarning() << "Tag not found" << tag;
    return false;
  }
  cdata->write(data);
  return true;
}

bool ServerBidiMethod::end(int tag) {
  auto cdata = remove(tag);
  if (!cdata) {
    qWarning() << "Tag not found" << tag;
    return false;
  }
  cdata->writesDone();
  return true;
}

ServerBidiCallData::ServerBidiCallData(
    ServerBidiMethod* method,
    GrpcService* service,
    int index,
    ::grpc::ServerCompletionQueue* cq,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write)
    : read_(read),
      write_(write),
      method_(method),
      stream_(&context_),
      cq_(cq),
      index_(index),
      service_(service) {
  process(true);
}

void ServerBidiCallData::process(bool ok) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (status_ == Status::INIT) {
    request_.reset(read_->newMessage());
    service_->raw()->RequestClientStreaming(index_, &context_, &stream_, cq_,
                                            cq_, this);
    status_ = Status::REQUEST;
  } else if (status_ == Status::REQUEST && !ok) {
    // init called after shutdown ?
    lock.unlock();
    delete this;
  } else if (status_ == Status::REQUEST) {
    // handleQueuedMessages();
    status_ = Status::READ;
    stream_.Read(request_.get(), this);
  } else if (status_ == Status::READ && ok) {
    method_->onData(this, read_->dataFromMessage(*request_));
    request_->Clear();
    stream_.Read(request_.get(), this);
  } else if (status_ == Status::READ) {
    // client streaming completed
    method_->onDataEnd(this);
    handleQueuedMessages();
    // status_ = Status::DONE;
    // stream_.Finish(grpc::Status::OK, this);
  } else if (status_ == Status::WRITE) {
    handleQueuedMessages();
  } else if (status_ == Status::DONE) {
    new ServerBidiCallData(method_, service_, index_, cq_, read_, write_);
    if (!ok) {
      method_->unknownError(tag_, "Error while finishing call.");
    }
    lock.unlock();
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

void ServerBidiCallData::handleQueuedMessages() {
  if (queue_.empty()) {
    status_ = Status::FROZEN;
  } else {
    response_.swap(queue_.front());
    queue_.pop();
    if (!response_) {
      // status_ = Status::READ;
      // stream_.Read(request_.get(), this);
      status_ = Status::DONE;
      stream_.Finish(grpc::Status::OK, this);
    } else {
      status_ = Status::WRITE;
      stream_.Write(*response_, this);
    }
  }
}

void ServerBidiCallData::write(const QVariant& data) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (status_ == Status::FROZEN) {
    response_.reset(write_->dataToMessage(data));
    if (!response_) {
      qWarning() << "Failed to serialize message to send.";
      return;
    }
    status_ = Status::WRITE;
    stream_.Write(*response_, this);
  } else {
    queue_.emplace(write_->dataToMessage(data));
  }
}

void ServerBidiCallData::writesDone() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (status_ == Status::FROZEN) {
    // status_ = Status::READ;
    // stream_.Read(request_.get(), this);
    status_ = Status::DONE;
    stream_.Finish(grpc::Status::OK, this);
  } else {
    queue_.emplace(nullptr);
  }
}
}
}
