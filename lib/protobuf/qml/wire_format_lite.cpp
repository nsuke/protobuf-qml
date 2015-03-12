#include "protobuf/qml/wire_format_lite.h"
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/coded_stream.h>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

void QmlWireFormatLite::clearError() {
  if (has_error_) {
    has_error_ = false;
  }
}

void QmlWireFormatLite::set_error(QString msg) {
  has_error_ = true;
  last_error_ = std::move(msg);
}

const QString& QmlWireFormatLite::error() const {
  return has_error_ ? last_error_ : empty_error_;
}

qint32 QmlWireFormatLite::readInt32(InputDevice* input) {
  clearError();
  int32 v = 0;
  try {
    if (!input) {
      set_error("input is null");
    } else {
      auto s = input->createSession();
      if (!s.stream()) {
        set_error("data not available");
      } else {
        io::CodedInputStream stream(s.stream());
        if (!internal::WireFormatLite::ReadPrimitive<
                int32,
                internal::WireFormatLite::TYPE_INT32>(&stream, &v)) {
          set_error("Failed to parse");
        }
      }
    }
  } catch (std::exception& ex) {
    set_error(ex.what());
  }
  return static_cast<qint32>(v);
}

void QmlWireFormatLite::writeInt32(OutputDevice* output,
                                   int tag,
                                   qint32 value) {
  clearError();
  if (!output) {
    set_error("output is null");
  } else {
    auto s = output->createSession();
    if (!s.stream()) {
      set_error("data not available");
    } else {
      io::CodedOutputStream stream(s.stream());
      internal::WireFormatLite::WriteInt32(
          tag, static_cast<int32>(value), &stream);
    }
  }
}
}
}
