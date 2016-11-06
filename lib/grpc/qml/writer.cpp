#include "grpc/qml/writer.h"
#include "grpc/qml/client_calldata.h"
#include "grpc/qml/logging.h"

#include <grpc++/impl/codegen/proto_utils.h>

#include <queue>

namespace grpc {
namespace qml {

class WriterCallData final : public ClientCallData<WriterCallData> {
public:
  WriterCallData(int tag,
                 grpc::Channel* channel,
                 ::grpc::CompletionQueue* cq,
                 WriterMethod* method,
                 ::protobuf::qml::DescriptorWrapper* read);

  ~WriterCallData();

  int tag() const { return tag_; }
  WriterMethod* method() { return method_; }

  void process(bool ok) override;

  bool write(std::unique_ptr<google::protobuf::Message> request);
  bool writesDone();
  int timeout() const { return timeout_; }
  void set_timeout(int timeout);

private:
  enum class Status {
    INIT,
    FROZEN,
    WRITE,
    DONE,
    FINISH,
  };
  void handleQueuedRequests();

  int timeout_ = -1;
  std::mutex mutex_;
  Status status_ = Status::INIT;
  ::grpc::CompletionQueue* cq_;
  ::grpc::ClientContext context_;
  grpc::Channel* channel_;
  ::protobuf::qml::DescriptorWrapper* read_;
  WriterMethod* method_;
  int tag_;
  std::queue<std::unique_ptr<google::protobuf::Message>> requests_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::shared_ptr<google::protobuf::Message> response_;
  grpc::Status grpc_status_;
  grpc::ClientAsyncWriter<google::protobuf::Message> writer_;
};

WriterCallData::WriterCallData(int tag,
                               grpc::Channel* channel,
                               ::grpc::CompletionQueue* cq,
                               WriterMethod* method,
                               ::protobuf::qml::DescriptorWrapper* read)
    : ClientCallData<WriterCallData>(this),
      cq_(cq),
      channel_(channel),
      read_(read),
      method_(method),
      tag_(tag),
      response_(read_->newMessage()),
      writer_(channel_, cq_, method_->raw(), &context_, response_.get(), this) {
}

WriterCallData::~WriterCallData() {}

void WriterCallData::process(bool ok) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (status_ == Status::INIT) {
    MSG_DEBUG << "INIT" << ok;
    if (!ok) {
      status_ = Status::FINISH;
      writer_.Finish(&grpc_status(), this);
    } else {
      handleQueuedRequests();
    }
  } else if (status_ == Status::WRITE) {
    MSG_DEBUG << "WRITE" << ok;
    if (!ok || !grpc_status().ok()) {
      reportGrpcError("Failed to send message");
      // TODO: should we abort current call entirely ?
    }
    handleQueuedRequests();
  } else if (status_ == Status::DONE) {
    MSG_DEBUG << "DONE" << ok;
    if (!ok || !grpc_status().ok()) {
      reportGrpcError("Failed to send client streaming done.");
    }
    status_ = Status::FINISH;
    writer_.Finish(&grpc_status(), this);
  } else if (status_ == Status::FINISH) {
    MSG_DEBUG << "FINISH" << ok;
    if (!ok || !grpc_status().ok()) {
      reportGrpcError("Error while finishing.");
    } else {
      method_->data(tag_, response_);
      response_.reset(read_->newMessage());
    }
    method_->deleteCall(tag_);
    // Release member mutex before deleting itself.
    lock.unlock();
    delete this;
  } else {
    MSG_DEBUG << "UNKNOWN" << ok;
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
      status_ = Status::WRITE;
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

WriterMethod::WriterMethod(const std::string& name,
                           ::protobuf::qml::DescriptorWrapper* read,
                           ::protobuf::qml::DescriptorWrapper* write,
                           std::shared_ptr<grpc::Channel> channel,
                           grpc::CompletionQueue* cq,
                           QObject* p)
    : ::protobuf::qml::WriterMethod(p),
      name_(name),
      read_(read),
      write_(write),
      cq_(cq),
      channel_(std::move(channel)),
      raw_(name.c_str(), grpc::RpcMethod::CLIENT_STREAMING, channel_) {}

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
  decltype(ensureCallData(tag)) call;
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    call = ensureCallData(tag);
  }
  if (!call) return false;
  return call->write(std::move(request));
}

bool WriterMethod::writesDone(int tag) {
  decltype(calls_.find(tag)->second) call;
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    auto it = calls_.find(tag);
    if (it == calls_.end()) {
      qWarning() << "Tag not found for writesDone " << tag;
      return false;
    }
    call = it->second;
  }
  return call->writesDone();
}

int WriterMethod::timeout(int tag) const {
  decltype(calls_.find(tag)->second) call;
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    const auto it = calls_.find(tag);
    if (it == calls_.end()) {
      qWarning() << "Tag not found for timeout " << tag;
      return false;
    }
    call = it->second;
  }
  return call->timeout();
}

void WriterMethod::set_timeout(int tag, int milliseconds) {
  decltype(ensureCallData(tag)) call;
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    call = ensureCallData(tag);
  }
  if (!call) return;
  call->set_timeout(milliseconds);
}

void WriterMethod::deleteCall(int tag) {
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    auto it = calls_.find(tag);
    if (it == calls_.end()) {
      qWarning() << "Tag not found for deleteCall " << tag;
      return;
    }
    calls_.erase(it);
  }
  closed(tag);
}
}
}
