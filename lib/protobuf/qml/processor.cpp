#include "protobuf/qml/processor.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

void Processor::write(int tag, const QVariant& data) {
  max_tag_ = std::max(tag, max_tag_);
  if (!desc_) {
    qWarning("Descriptor is null");
    error(tag, "Descriptor is null.");
    return;
  }
  auto msg = desc_->dataToMessage(data);
  if (!msg) {
    error(tag, "Failed to convert message data to protobuf message object.");
    return;
  }
  doWrite(tag, *msg);
}

void GenericStreamProcessor::doRead(int tag) {
  auto close = std::bind(&GenericStreamProcessor::closeInput, this, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyInputStream, decltype(close)> stream(
      openInput(tag), close);
  if (!stream) {
    error(tag, "Failed to open input stream");
    return;
  }
  std::unique_ptr<google::protobuf::Message> msg(descriptor()->newMessage());
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

void GenericStreamProcessor::doWrite(int tag,
                                     const google::protobuf::Message& msg) {
  auto close = std::bind(&GenericStreamProcessor::closeOutput, this, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyOutputStream, decltype(close)> stream(
      openOutput(tag, msg.ByteSize()), close);
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

void GenericStreamProcessor::closeInput(
    int tag, google::protobuf::io::ZeroCopyInputStream* stream) {
  if (stream) delete stream;
}
void GenericStreamProcessor::closeOutput(
    int tag, google::protobuf::io::ZeroCopyOutputStream* stream) {
  if (stream) delete stream;
}
}
}
