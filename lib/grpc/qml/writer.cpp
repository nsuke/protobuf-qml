#include "grpc/qml/writer.h"

#include <iostream>

namespace grpc {
namespace qml {

WriterCallData::WriterCallData(int tag,
                               grpc::Channel* channel,
                               ::grpc::CompletionQueue* cq,
                               WriterMethod* method,
                               ::protobuf::qml::DescriptorWrapper* read)
    : cq_(cq),
      channel_(channel),
      read_(read),
      method_(method),
      tag_(tag),
      response_(read_->newMessage()),
      writer_(channel_, cq_, method_->raw(), &context_, response_.get(), this) {
}

WriterCallData::~WriterCallData() {
}

void WriterCallData::process(bool ok) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (status_ == Status::INIT) {
    Q_ASSERT(ok);
    handleQueuedRequests();
  } else if (status_ == Status::WRITE) {
    if (!ok) {
      qWarning() << QString::fromStdString(grpc_status_.error_message());
      method_->unknownError(tag_, "Failed to send message.");
      // TODO: should we abort this call entirely ?
    }
    handleQueuedRequests();
  } else if (status_ == Status::DONE) {
    if (!ok) {
      qWarning() << QString::fromStdString(grpc_status_.error_message());
      method_->unknownError(tag_, "Failed to send client streaming done.");
    }
    status_ = Status::FINISH;
    writer_.Finish(&grpc_status_, this);
  } else if (status_ == Status::FINISH) {
    if (ok) {
      method_->data(tag_, response_);
      response_.reset(read_->newMessage());
    } else {
      method_->error(tag_, grpc_status_.error_code(),
                     QString::fromStdString(grpc_status_.error_message()));
    }
    method_->deleteCall(tag_);
    // Release member mutex before deleting itself.
    lock.unlock();
    delete this;
  } else {
    qWarning() << "Unexpected status : " << static_cast<int>(status_);
    Q_ASSERT(false);
  }
}

void WriterCallData::handleQueuedRequests() {
  if (requests_.empty()) {
    status_ = Status::FROZEN;
  } else {
    request_.swap(requests_.front());
    requests_.pop();
    if (!request_) {
      status_ = Status::DONE;
      writer_.WritesDone(this);
    } else {
      writer_.Write(*request_, this);
    }
  }
}

bool WriterCallData::write(std::unique_ptr<google::protobuf::Message> request) {
  if (!request) {
    method_->unknownError(tag_, "Request message is empty.");
    return false;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  if (status_ >= Status::DONE) {
    qWarning() << "write called for finishing writer call.";
    return false;
  } else if (status_ == Status::FROZEN) {
    request_.swap(request);
    status_ = Status::WRITE;
    writer_.Write(*request_, this);
  } else {
    // Enqueue awaiting requests because underlying gRPC call object does not
    // allow more than one request at once.
    requests_.push(std::move(request));
  }
  return true;
}

bool WriterCallData::writesDone() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (status_ >= Status::DONE) {
    qWarning() << "writesDone called for finishing writer call.";
    return false;
  } else if (status_ == Status::FROZEN) {
    status_ = Status::DONE;
    writer_.WritesDone(this);
  } else {
    requests_.emplace(nullptr);
  }
  return true;
}

void WriterCallData::set_timeout(int timeout) {
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

WriterCallData* WriterMethod::ensureCallData(int tag) {
  WriterCallData* call = nullptr;
  auto it = calls_.find(tag);
  if (it == calls_.end()) {
    auto res = calls_.insert(std::make_pair(
        tag, new WriterCallData(tag, channel_.get(), cq_, this, read_)));
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

bool WriterMethod::write(int tag,
                         std::unique_ptr<google::protobuf::Message> request) {
  if (!request) {
    unknownError(tag, "Failed to convert to message object.");
    return false;
  }
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto call = ensureCallData(tag);
  if (!call) return false;
  return call->write(std::move(request));
}

bool WriterMethod::writesDone(int tag) {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto it = calls_.find(tag);
  if (it == calls_.end()) {
    qWarning() << "Tag not found for writesDone " << tag;
    return false;
  }
  return it->second->writesDone();
}

int WriterMethod::timeout(int tag) const {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  const auto it = calls_.find(tag);
  if (it == calls_.end()) {
    qWarning() << "Tag not found for timeout " << tag;
    return false;
  }
  return it->second->timeout();
}

void WriterMethod::set_timeout(int tag, int milliseconds) {
  std::lock_guard<std::mutex> lock(calls_mutex_);
  auto call = ensureCallData(tag);
  if (!call) return;
  call->set_timeout(milliseconds);
}

void WriterMethod::deleteCall(int tag) {
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
