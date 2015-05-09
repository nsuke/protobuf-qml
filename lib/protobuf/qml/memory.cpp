#include "protobuf/qml/memory.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

void MemoryBuffer::set_block_size(int block_size) {
  if (block_size_ != block_size) {
    block_size_ = block_size;
    blockSizeChanged();
  }
}

void MemoryBuffer::set_size(int size) {
  if (size_ != size) {
    size_ = size;
    buffer_.resize(size_ >= 0 ? size_ : 0);
    sizeChanged();
  }
}

google::protobuf::io::ZeroCopyInputStream* MemoryBuffer::openInput(int tag) {
  return new google::protobuf::io::ArrayInputStream(buffer_.data(),
                                                    buffer_.size());
}

google::protobuf::io::ZeroCopyOutputStream* MemoryBuffer::openOutput(int tag,
                                                                     int hint) {
  buffer_.resize(hint);
  return new google::protobuf::io::ArrayOutputStream(buffer_.data(),
                                                     buffer_.size());
}
}
}
