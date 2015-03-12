#include "protobuf/qml/memory.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

MemoryInput::SessionPtr MemoryInput::createSession() {
  return SessionPtr(this, impl_.createStream());
}

MemoryOutput::SessionPtr MemoryOutput::createSession() {
  return SessionPtr(this, impl_.createStream());
}

void MemoryBuffer::set_block_size(int block_size) {
  if (block_size_ != block_size) {
    block_size_ = block_size;
    input_.set_block_size(block_size);
    output_.set_block_size(block_size);
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
}
}
