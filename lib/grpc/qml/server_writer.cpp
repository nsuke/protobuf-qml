#include "grpc/qml/server_writer.h"
#include "grpc/qml/server.h"

namespace grpc {
namespace qml {

ServerWriterMethod::ServerWriterMethod(
    GrpcService* service,
    int index,
    ::grpc::ServerCompletionQueue* cq,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write)
    : read_(read), write_(write), cq_(cq), index_(index), service_(service) {
}

void ServerWriterMethod::startProcessing() {
  new ServerWriterCallData(this, service_, index_, cq_, read_, write_);
}

void ServerWriterMethod::onRequest(ServerWriterCallData* cdata,
                                   const QVariant& v) {
  auto tag = store(cdata);
  data(tag, v);
}

bool ServerWriterMethod::respond(int tag, const QVariant& data) {
  auto lk = lock();
  auto cdata = getUnsafe(tag);
  if (!cdata) {
    qWarning() << "";
    return false;
  }
  cdata->write(data);
  return true;
}

bool ServerWriterMethod::end(int tag) {
  auto cdata = remove(tag);
  if (!cdata) {
    qWarning() << "end called for unknown tag" << tag;
    return false;
  }
  cdata->end();
  return true;
}

ServerWriterCallData::ServerWriterCallData(
    ServerWriterMethod* method,
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

void ServerWriterCallData::process(bool ok) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (status_ == Status::INIT) {
    request_.reset(read_->newMessage());
    service_->raw()->RequestServerStreaming(index_, &context_, request_.get(),
                                            &writer_, cq_, cq_, this);
    status_ = Status::READ;
  } else if (status_ == Status::READ) {
    if (!ok) {
      // init called after shutdown ?
      lock.unlock();
      delete this;
      return;
    }
    auto data = read_->dataFromMessage(*request_);
    method_->onRequest(this, data);
    processQueuedData();
  } else if (status_ == Status::WRITE) {
    if (!ok) {
      qWarning() << "Failed to write.";
      method_->error(tag_, "Failed to write.");
    }
    processQueuedData();
  } else if (status_ == Status::DONE) {
    new ServerWriterCallData(method_, service_, index_, cq_, read_, write_);
    if (!ok) {
      qWarning() << "Failed to complete writer call.";
      method_->error(tag_, "Failed to complete writer call.");
    }
    lock.unlock();
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

// Should only be called from IO thread.
void ServerWriterCallData::processQueuedData() {
  if (queue_.empty()) {
    status_ = Status::FROZEN;
  } else {
    response_.swap(queue_.front());
    queue_.pop();
    if (!response_) {
      // nullptr has been put to signify the end of this call.
      status_ = Status::DONE;
      writer_.Finish(grpc::Status::OK, this);
    } else {
      // usual send
      status_ = Status::WRITE;
      writer_.Write(*response_, this);
    }
  }
}

void ServerWriterCallData::write(const QVariant& data) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (status_ == Status::FROZEN) {
    // Since it's been frozen, we can safely delete existing response object.
    response_.reset(write_->dataToMessage(data));
    if (!response_) {
      // TODO: Should we abort this call ?
      qWarning() << "Failed to create message object to write.";
      return;
    }
    status_ = Status::WRITE;
    writer_.Write(*response_, this);
  } else {
    enqueueData(data);
  }
}

// Should only be called from inside locked scope.
void ServerWriterCallData::enqueueData(const QVariant& data) {
  auto rsp = write_->dataToMessage(data);
  if (!rsp) {
    qWarning() << "Failed to create message object to store to send later."
               << "The message is ignored.";
    return;
  }
  queue_.emplace(rsp);
}

void ServerWriterCallData::end() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (status_ == Status::FROZEN) {
    status_ = Status::DONE;
    writer_.Finish(grpc::Status::OK, this);
  } else {
    // Let's put a poison pill to kill it later.
    queue_.push(nullptr);
  }
}
}
}
