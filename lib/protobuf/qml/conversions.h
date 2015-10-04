#ifndef PROTOBUF_QML_CONVERSIONS_H
#define PROTOBUF_QML_CONVERSIONS_H

#include "protobuf/qml/common.h"
#include <google/protobuf/message.h>
#include <QObject>
#include <QJSValue>
#include <memory>
#include <stdexcept>

namespace QV4 {
class ExecutionEngine;
}

namespace protobuf {
namespace qml {

class PROTOBUF_QML_DLLEXPORT JSValueConverter {
public:
  static std::unique_ptr<JSValueConverter> On(const QObject* o);
  static std::unique_ptr<JSValueConverter> For(const QJSValue& o);

  QJSValue toJSValue(const google::protobuf::Message& message);
  bool fromJSValue(google::protobuf::Message& message, const QJSValue& value);

private:
  JSValueConverter(QV4::ExecutionEngine* engine) : engine_(engine) {
    if (!engine_) throw std::invalid_argument("Engine is null");
  }
  QV4::ExecutionEngine* engine_;
};
}
}

#endif  // PROTOBUF_QML_CONVERSIONS_H
