#include "protobuf/qml/processor.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

namespace detail {
Worker::Worker(Processor* p) {
  moveToThread(&thread_);
  connect(this, &Worker::write,
          [p](int tag, const QVariant& data) { p->writeThread(tag, data); });
  connect(this, &Worker::writeEmpty,
          [p](int tag) { p->writeEmptyThread(tag); });
  connect(this, &Worker::writeEnd, [p](int tag) { p->writeEndThread(tag); });
  thread_.start();
}
}

void Processor::write(int tag, const QVariant& data) {
  max_tag_ = std::max(tag, max_tag_);
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

void Processor::writeThread(int tag, const QVariant& data) {
  if (!write_desc_) {
    qWarning("Descriptor is null");
    error(tag, "Descriptor is null.");
    return;
  }
  auto msg = write_desc_->dataToMessage(data);
  if (!msg) {
    error(tag, "Failed to convert message data to protobuf message object.");
    return;
  }
  doWrite(tag, *msg);
}

// Serialize protobuf message to underlying buffer
void BufferMethod::doWrite(int tag, const google::protobuf::Message& msg) {
  if (!buffer_) {
    error(tag, "Buffer object is not available.");
    return;
  }
  auto close = std::bind(&BufferChannel::closeOutput, buffer_, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyOutputStream, decltype(close)> stream(
      buffer_->openOutput(tag, msg.ByteSize()), close);
  if (!stream) {
    error(tag, "Failed to open output stream");
    return;
  }
  if (!msg.SerializeToZeroCopyStream(stream.get())) {
    error(tag, "Failed to serialize to protobuf stream");
    return;
  }
  // Flush before notifying "done".
  stream.reset();
  data(tag, QVariant());
}

// Parse protobuf message from underlying buffer
void BufferMethod::doEmptyWrite(int tag) {
  if (!buffer_) {
    error(tag, "Buffer object is not available.");
    return;
  }
  auto close = std::bind(&BufferChannel::closeInput, buffer_, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyInputStream, decltype(close)> stream(
      buffer_->openInput(tag), close);
  if (!stream) {
    error(tag, "Failed to open input stream");
    return;
  }
  std::unique_ptr<google::protobuf::Message> msg(
      read_descriptor()->newMessage());
  if (!msg) {
    error(tag, "Failed to create protobuf message object");
    return;
  }
  if (!msg->ParseFromZeroCopyStream(stream.get())) {
    error(tag, "Failed to parse from protobuf buffer");
    return;
  }
  stream.reset();
  emitMessageData(tag, *msg);
}
}
}
