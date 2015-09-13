#include "grpc/qml/reader.h"

namespace grpc {
namespace qml {

ReaderCallData::ReaderCallData(
    int tag,
    grpc::Channel* channel,
    grpc::CompletionQueue* cq,
    ReaderMethod* method,
    ::protobuf::qml::DescriptorWrapper* read,
    std::unique_ptr<google::protobuf::Message> request,
    int timeout)
    : cq_(cq),
      channel_(channel),
      read_(read),
      method_(method),
      tag_(tag),
      request_(std::move(request)),
      response_(read_->newMessage()) {
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  }
  std::lock_guard<std::mutex> lock(mutex_);
  reader_.reset(new grpc::ClientAsyncReader<google::protobuf::Message>(
      channel_, cq_, method_->raw(), &context_, *request_, this));
}

ReaderCallData::~ReaderCallData() {
}

void ReaderCallData::process(bool ok) {
  if (status_ == Status::WRITE) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
    }
    if (!ok) {
      method_->unknownError(tag_, "Failed to send request.");
      // TODO: need to call Finish or not ?
      delete this;
      return;
    }
    status_ = Status::READ;
    reader_->Read(response_.get(), this);
  } else if (status_ == Status::READ) {
    if (!ok) {
      method_->dataEnd(tag_);
      status_ = Status::DONE;
      reader_->Finish(&grpc_status_, this);
    } else {
      method_->data(tag_, response_);
      response_.reset(read_->newMessage());
      reader_->Read(response_.get(), this);
    }
  } else if (status_ == Status::DONE) {
    if (grpc_status_.error_code()) {
      qWarning() << QString::fromStdString(grpc_status_.error_message());
      method_->error(tag_, grpc_status_.error_code(),
                     QString::fromStdString(grpc_status_.error_message()));
    }
    method_->closed(tag_);
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

bool ReaderMethod::write(int tag,
                         std::unique_ptr<google::protobuf::Message> request,
                         int timeout) {
  if (!request) {
    unknownError(tag, "Failed to convert to message object.");
    return false;
  }
  new ReaderCallData(tag, channel_.get(), cq_, this, read_, std::move(request),
                     timeout);
  return true;
}
}
}
