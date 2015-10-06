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
  // Create a converter instance using Javascript engine retrived from QObject.
  // The QObject needs to be instantiated by QML engine, otherwise it returns
  // nullptr.
  static std::unique_ptr<JSValueConverter> fromQObject(const QObject* o);

  // Create a converter instance using Javascript engine retrived from QJSValue.
  // Returns nullptr if the eingine is not available.
  static std::unique_ptr<JSValueConverter> fromQJSValue(const QJSValue& o);

  // Convert a standard protobuf message instance to internal Javascript
  // structure of protobuf-qml. Currently, the format is opaque and users need
  // to pass this object to the constructor of the message type in
  // Javascript (QML) side.
  QJSValue toJSValue(const google::protobuf::Message& message);

  // Convert a protobuf-qml Javascript message object into standard C++ message
  // object.
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
