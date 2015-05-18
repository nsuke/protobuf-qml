#include "protobuf/qml/processor.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

void Channel::emitMessageData(Method* method,
                              int tag,
                              const google::protobuf::Message& msg) {
  method->emitMessageData(tag, msg);
}

namespace detail {
Worker::Worker(Method* p) {
  moveToThread(&thread_);
  connect(this, &Worker::write,
          [p](int tag, const QVariant& data) { p->writeThread(tag, data); });
  connect(this, &Worker::writeEmpty,
          [p](int tag) { p->writeEmptyThread(tag); });
  connect(this, &Worker::writeEnd, [p](int tag) { p->writeEndThread(tag); });
  thread_.start();
}
}

void Method::write(int tag, const QVariant& data) {
  max_tag_ = std::max(tag, max_tag_);
  if (!channel_) {
    error(tag, "Channel is null.");
    return;
  }
  if (data.isValid()) {
    if (!write_desc_) {
      qWarning("Descriptor is null");
      error(tag, "Descriptor is null.");
      return;
    }
    if (async_) {
      ensureWorker();
      worker_->write(tag, data);
    } else {
      writeThread(tag, data);
    }
  } else {
    if (async_) {
      ensureWorker();
      worker_->writeEmpty(tag);
    } else {
      writeEmptyThread(tag);
    }
  }
}

void Method::writeEnd(int tag) {
  max_tag_ = std::max(tag, max_tag_);
  if (!channel_) {
    error(tag, "Channel is null.");
    return;
  }
  // TODO: Need to check write desc here ?
  // if (!write_desc_) {
  //   error(tag, "Descriptor is null.");
  //   return;
  // }
  if (async_) {
    ensureWorker();
    worker_->writeEnd(tag);
  } else {
    writeEndThread(tag);
  }
}

void Method::writeThread(int tag, const QVariant& data) {
  if (!write_desc_) {
    qWarning("Descriptor is null.");
    error(tag, "Descriptor is null.");
    return;
  }
  auto msg = write_desc_->dataToMessage(data);
  if (!msg) {
    error(tag, "Failed to convert message data to protobuf message object.");
    return;
  }
  channel_->write(this, tag, *msg);
}

void BufferChannel::write(Method* method,
                          int tag,
                          const google::protobuf::Message& msg) {
  auto close =
      std::bind(&BufferChannel::closeOutput, this, tag, std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyOutputStream, decltype(close)> stream(
      this->openOutput(tag, msg.ByteSize()), close);
  if (!stream) {
    method->error(tag, "Failed to open output stream");
    return;
  }
  if (!msg.SerializeToZeroCopyStream(stream.get())) {
    method->error(tag, "Failed to serialize to protobuf stream");
    return;
  }
  // Flush before notifying "done".
  stream.reset();
  method->data(tag, QVariant());
}

void BufferChannel::writeEmpty(Method* method, int tag) {
  auto close =
      std::bind(&BufferChannel::closeInput, this, tag, std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyInputStream, decltype(close)> stream(
      this->openInput(tag), close);
  if (!stream) {
    method->error(tag, "Failed to open input stream");
    return;
  }
  std::unique_ptr<google::protobuf::Message> msg(
      method->read_descriptor()->newMessage());
  if (!msg) {
    method->error(tag, "Failed to create protobuf message object");
    return;
  }
  if (!msg->ParseFromZeroCopyStream(stream.get())) {
    method->error(tag, "Failed to parse from protobuf buffer");
    return;
  }
  stream.reset();
  emitMessageData(method, tag, *msg);
}
}
}
