#include "protobuf/qml/memory.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

void MemoryBuffer2::doRead(int tag) {
  io::ArrayInputStream stream(buffer_.data(), buffer_.size(),
                              effective_block_size());
  std::unique_ptr<google::protobuf::Message> msg(descriptor()->newMessage());
  if (!msg) {
    error(tag, "Failed to create protobuf message object");
    return;
  }
  if (!msg->ParseFromZeroCopyStream(&stream)) {
    error(tag, "Failed to parse from protobuf buffer");
    return;
  }
  emitMessageData(tag, *msg);
}

void MemoryBuffer2::doWrite(int tag, const google::protobuf::Message& msg) {
  buffer_.resize(msg.ByteSize());
  io::ArrayOutputStream stream(buffer_.data(), buffer_.size(),
                               effective_block_size());
  if (!msg.SerializeToZeroCopyStream(&stream)) {
    error(tag, "Failed to serialize to protobuf stream");
    return;
  }
  data(tag, QVariant());
}

void MemoryBuffer2::set_block_size(int block_size) {
  if (block_size_ != block_size) {
    block_size_ = block_size;
    blockSizeChanged();
  }
}
void MemoryBuffer2::set_size(int size) {
  if (size_ != size) {
    size_ = size;
    buffer_.resize(size_ >= 0 ? size_ : 0);
    sizeChanged();
  }
}
}
}
