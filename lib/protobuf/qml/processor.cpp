#include "protobuf/qml/processor.h"

namespace protobuf {
namespace qml {

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
}
}
