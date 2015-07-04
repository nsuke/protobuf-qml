#include "protobuf/qml/buffer.h"

#include <functional>
#include <memory>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

SerializeMethod::SerializeMethod(BufferChannel* channel,
                                 DescriptorWrapper* descriptor)
    : channel_(channel), descriptor_(descriptor) {
}

bool SerializeMethod::write(int tag, const QVariant& v, int timeout) {
  auto msg = descriptor_->dataToMessage(v);
  if (!msg) {
    qWarning() << "Failed to create message object to write";
    return false;
  }
  auto close = std::bind(&BufferChannel::closeOutput, channel_, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyOutputStream, decltype(close)> stream(
      channel_->openOutput(tag, msg->ByteSize()), close);
  if (!stream) {
    error(tag, "Failed to open output stream");
    return false;
  }
  if (!msg->SerializeToZeroCopyStream(stream.get())) {
    error(tag, "Failed to serialize to protobuf stream");
    return false;
  }
  // Flush before notifying "done".
  stream.reset();
  data(tag, QVariant());
  return true;
}

ParseMethod::ParseMethod(BufferChannel* channel, DescriptorWrapper* descriptor)
    : channel_(channel), descriptor_(descriptor) {
}

bool ParseMethod::write(int tag, const QVariant& v, int timeout) {
  if (v.isValid()) {
    // qWarning() << "Received unexpected non-empty value.";
  }
  auto close = std::bind(&BufferChannel::closeInput, channel_, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyInputStream, decltype(close)> stream(
      channel_->openInput(tag), close);
  if (!stream) {
    error(tag, "Failed to open input stream");
    return false;
  }
  std::unique_ptr<google::protobuf::Message> msg(descriptor_->newMessage());
  if (!msg) {
    error(tag, "Failed to create protobuf message object");
    return false;
  }
  if (!msg->ParseFromZeroCopyStream(stream.get())) {
    error(tag, "Failed to parse from protobuf buffer");
    return false;
  }
  stream.reset();
  auto v2 = descriptor_->dataFromMessage(*msg);
  data(tag, v2);
  return true;
}

UnaryMethod* BufferChannel::registerUnaryMethod(const QString& name,
                                                DescriptorWrapper* read,
                                                DescriptorWrapper* write) {
  auto method_name = name.split("/").back();
  if (method_name == "Serialize") {
    return new SerializeMethod(this, write);
  } else if (method_name == "Parse") {
    return new ParseMethod(this, read);
  }
  qWarning() << "Unknown method: " << name;
  return nullptr;
}
}
}
