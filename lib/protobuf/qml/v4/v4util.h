#ifndef PROTOBUF_QML_V4_V4UTIL_H
#define PROTOBUF_QML_V4_V4UTIL_H

#include <private/qv8engine_p.h>
#include <private/qv4arraybuffer_p.h>

namespace protobuf {
namespace qml {

QV4::Heap::ArrayBuffer* allocate_array_buffer(QV4::ExecutionEngine* v4,
                                              size_t size);

QV4::ReturnedValue packCallbackObject(QV4::ExecutionEngine* v4,
                                      const QV4::Value& callback);

std::pair<QV4::ReturnedValue, QV4::ReturnedValue> unpackCallbackObject(
    QV4::ExecutionEngine* v4, QV4::ReturnedValue packed);
}
}
#endif  // PROTOBUF_QML_V4_V4UTIL_H
