#include "grpc/qml/reader.h"

namespace grpc {
namespace qml {

ReaderCallData::ReaderCallData(
    int tag,
    grpc::ChannelInterface* channel,
    grpc::CompletionQueue* cq,
    ReaderMethod* method,
    ::protobuf::qml::DescriptorWrapper* read,
    std::unique_ptr<google::protobuf::Message> request,
    int timeout)
    : tag_(tag),
      channel_(channel),
      cq_(cq),
      method_(method),
      read_(read),
      request_(std::move(request)),
      response_(read_->newMessage()) {
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  }
  reader_.reset(new grpc::ClientAsyncReader<google::protobuf::Message>(
      channel_, cq_, method_->raw(), &context_, *request_, this));
}

ReaderCallData::~ReaderCallData() {
}

void ReaderCallData::process(bool ok) {
  if (status_ == Status::WRITE) {
    if (!ok) {
      method_->error(tag_, "Failed to send request.");
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
      method_->data(tag_, read_->dataFromMessage(*response_));
      response_->Clear();
      reader_->Read(response_.get(), this);
    }
  } else if (status_ == Status::DONE) {
    if (grpc_status_.error_code()) {
      qWarning() << QString::fromStdString(grpc_status_.error_message());
    }
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

bool ReaderMethod::write(int tag, const QVariant& data, int timeout) {
  std::unique_ptr<google::protobuf::Message> request(
      write_->dataToMessage(data));
  if (!request) {
    error(tag, "Failed to convert to message object.");
    return false;
  }
  new ReaderCallData(tag, channel_.get(), cq_, this, read_, std::move(request),
                     timeout);
  return true;
}
}
}
