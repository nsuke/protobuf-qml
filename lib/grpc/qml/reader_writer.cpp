#include "grpc/qml/reader_writer.h"

namespace grpc {
namespace qml {

ReaderWriterCallData::ReaderWriterCallData(
    int tag,
    grpc::Channel* channel,
    grpc::CompletionQueue* cq,
    ReaderWriterMethod* method,
    ::protobuf::qml::DescriptorWrapper* read)
    : cq_(cq),
      channel_(channel),
      read_(read),
      method_(method),
      tag_(tag),
      response_(read_->newMessage()),
      stream_(channel_, cq_, method_->raw(), &context_, this) {
}

ReaderWriterCallData::~ReaderWriterCallData() {
}

void ReaderWriterCallData::process(bool ok) {
  if (status_ == Status::INIT) {
    if (!ok) {
      status_ = Status::FINISH;
      stream_.Finish(&grpc_status_, this);
    } else {
      handleQueuedRequests();
    }
  } else if (status_ == Status::WRITE) {
    if (!ok) {
      method_->unknownError(tag_, "Failed to send request.");
      status_ = Status::FINISH;
      stream_.Finish(&grpc_status_, this);
    } else {
      handleQueuedRequests();
    }
  } else if (status_ == Status::READ) {
    if (!ok) {
      method_->dataEnd(tag_);
      read_done_ = true;
      if (write_done_) {
        status_ = Status::FINISH;
        stream_.Finish(&grpc_status_, this);
      } else {
        handleQueuedRequests();
      }
    } else {
      method_->data(tag_, response_);
      response_.reset(read_->newMessage());
      handleQueuedRequests();
    }
  } else if (status_ == Status::WRITES_DONE) {
    if (!ok) {
      qWarning() << QString::fromStdString(grpc_status_.error_message());
      status_ = Status::FINISH;
      stream_.Finish(&grpc_status_, this);
    }
    write_done_ = true;
    if (read_done_) {
      status_ = Status::FINISH;
      stream_.Finish(&grpc_status_, this);
    } else {
      status_ = Status::READ;
      stream_.Read(response_.get(), this);
    }
  } else if (status_ == Status::FINISH) {
    if (!grpc_status_.ok()) {
      method_->error(tag_, grpc_status_.error_code(),
                     QString::fromStdString(grpc_status_.error_message()));
    }
    method_->deleteCall(tag_);
  } else {
    Q_ASSERT(false);
  }
}

void ReaderWriterCallData::handleQueuedRequests() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (requests_.empty()) {
    // if (read_done_) {
    // It seems that gRPC stream can only be read when client can block until
    // next data.
    if (!write_done_) {
      status_ = Status::FROZEN;
    } else {
      status_ = Status::READ;
      stream_.Read(response_.get(), this);
    }
  } else {
    request_.swap(requests_.front());
    requests_.pop();
    if (!request_) {
      status_ = Status::WRITES_DONE;
      stream_.WritesDone(this);
    } else {
      status_ = Status::WRITE;
      stream_.Write(*request_, this);
    }
  }
}

bool ReaderWriterCallData::write(
    std::unique_ptr<google::protobuf::Message> request) {
  if (!request) {
    method_->unknownError(tag_, "Request message is empty.");
    return false;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  if (write_done_queued_) {
    qWarning() << "write called for finishing writer call.";
    return false;
  } else if (status_ == Status::FROZEN) {
    request_.swap(request);
    status_ = Status::WRITE;
    stream_.Write(*request_, this);
  } else {
    // Enqueue awaiting requests because underlying gRPC call object does not
    // allow more than one request at once.
    requests_.push(std::move(request));
  }
  return true;
}

bool ReaderWriterCallData::writesDone() {
  std::lock_guard<std::mutex> lock(mutex_);
  write_done_queued_ = true;
  if (status_ >= Status::WRITES_DONE) {
    qWarning() << "writesDone called for finishing writer call.";
    return false;
  } else if (status_ == Status::FROZEN) {
    status_ = Status::WRITES_DONE;
    stream_.WritesDone(this);
  } else {
    requests_.emplace(nullptr);
  }
  return true;
}

void ReaderWriterCallData::set_timeout(int timeout) {
  timeout_ = timeout;
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  } else {
    // Set to quasi-infinity
    context_.set_deadline(
        std::chrono::time_point<std::chrono::system_clock>::max());
  }
}

ReaderWriterCallData* ReaderWriterMethod::ensureCallData(int tag) {
  ReaderWriterCallData* call = nullptr;
  auto it = calls_.find(tag);
  if (it == calls_.end()) {
    auto res = calls_.insert(std::make_pair(
        tag, new ReaderWriterCallData(tag, channel_.get(), cq_, this, read_)));
    if (!res.second) {
      unknownError(tag, "Failed to create call object");
      return nullptr;
    }
    call = res.first->second;
  } else {
    call = it->second;
  }
  return call;
}

bool ReaderWriterMethod::call(int tag) {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  return ensureCallData(tag);
}

bool ReaderWriterMethod::write(
    int tag, std::unique_ptr<google::protobuf::Message> request) {
  if (!request) {
    unknownError(tag, "Failed to convert to message object.");
    return false;
  }
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto it = calls_.find(tag);
  if (it == calls_.end()) {
    qWarning() << "Tag not found for write " << tag;
    return false;
  }
  return it->second->write(std::move(request));
}

bool ReaderWriterMethod::writesDone(int tag) {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto it = calls_.find(tag);
  if (it == calls_.end()) {
    qWarning() << "Tag not found for writesDone " << tag;
    return false;
  }
  return it->second->writesDone();
}

int ReaderWriterMethod::timeout(int tag) const {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  const auto it = calls_.find(tag);
  if (it == calls_.end()) {
    qWarning() << "Tag not found for timeout " << tag;
    return false;
  }
  return it->second->timeout();
}

void ReaderWriterMethod::set_timeout(int tag, int milliseconds) {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto call = ensureCallData(tag);
  if (!call) return;
  call->set_timeout(milliseconds);
}

void ReaderWriterMethod::deleteCall(int tag) {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto it = calls_.find(tag);
  if (it == calls_.end()) {
    qWarning() << "Tag not found for deleteCall " << tag;
    return;
  }
  calls_.erase(it);
  closed(tag);
}
}
}
