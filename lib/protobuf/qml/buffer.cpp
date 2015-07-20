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

bool SerializeMethod::write(int tag,
                            std::unique_ptr<google::protobuf::Message> msg,
                            int timeout) {
  if (!msg) {
    qWarning() << "No message object to write";
    return false;
  }
  auto close = std::bind(&BufferChannel::closeOutput, channel_, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyOutputStream, decltype(close)> stream(
      channel_->openOutput(tag, msg->ByteSize()), close);
  if (!stream) {
    unknownError(tag, "Failed to open output stream");
    return false;
  }
  if (!msg->SerializeToZeroCopyStream(stream.get())) {
    unknownError(tag, "Failed to serialize to protobuf stream");
    return false;
  }
  // Flush before notifying "done".
  stream.reset();
  data(tag, 0);
  return true;
}

ParseMethod::ParseMethod(BufferChannel* channel, DescriptorWrapper* descriptor)
    : channel_(channel), descriptor_(descriptor) {
}

bool ParseMethod::write(int tag,
                        std::unique_ptr<google::protobuf::Message>,
                        int timeout) {
  // if (v.isValid()) {
  //   // qWarning() << "Received unexpected non-empty value.";
  // }
  auto close = std::bind(&BufferChannel::closeInput, channel_, tag,
                         std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyInputStream, decltype(close)> stream(
      channel_->openInput(tag), close);
  if (!stream) {
    unknownError(tag, "Failed to open input stream");
    return false;
  }
  std::shared_ptr<google::protobuf::Message> msg(descriptor_->newMessage());
  if (!msg) {
    unknownError(tag, "Failed to create protobuf message object");
    return false;
  }
  if (!msg->ParseFromZeroCopyStream(stream.get())) {
    unknownError(tag, "Failed to parse from protobuf buffer");
    return false;
  }
  stream.reset();
  data(tag, msg);
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
