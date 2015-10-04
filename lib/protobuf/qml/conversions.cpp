#include "protobuf/qml/conversions.h"
#include "protobuf/qml/v4/descriptor.h"
#include <private/qqmlengine_p.h>
#include <private/qjsvalue_p.h>

using namespace QV4;

namespace protobuf {
namespace qml {

std::unique_ptr<JSValueConverter> JSValueConverter::On(const QObject* o) {
  auto engine = QQmlEnginePrivate::getV4Engine(qmlEngine(o));
  if (!engine) {
    qWarning() << "Failed to retrieve QML engine from QObject. Make sure it is "
                  "instantiated by QML engine.";
    return nullptr;
  }
  return std::unique_ptr<JSValueConverter>(new JSValueConverter(engine));
}

std::unique_ptr<JSValueConverter> JSValueConverter::For(const QJSValue& o) {
  auto engine = QJSValuePrivate::engine(&o);
  if (!engine) {
    qWarning() << "Failed to retrieve QML engine from QJSValue.";
    return nullptr;
  }
  return std::unique_ptr<JSValueConverter>(new JSValueConverter(engine));
}

QJSValue JSValueConverter::toJSValue(const google::protobuf::Message& message) {
  Q_ASSERT(engine_);
  auto desc = message.GetDescriptor();
  if (!desc) {
    qWarning() << "Failed to retrieve message descriptor.";
    return QJSValue();
  }
  Descriptor d(desc);

  return QJSValue(engine_, d.messageToJsValue(engine_, message));
}

bool JSValueConverter::fromJSValue(google::protobuf::Message& message,
                                   const QJSValue& value) {
  Q_ASSERT(engine_);
  auto desc = message.GetDescriptor();
  if (!desc) {
    qWarning() << "Failed to retrieve message descriptor.";
    return false;
  }
  Descriptor d(desc);

  if (!value.hasProperty("_raw")) {
    qWarning() << "QJSValue does not seem to be a JS Protobuf message.";
    return false;
  }
  auto raw = value.property("_raw");
  if (!raw.isArray()) {
    qWarning() << "QJSValue does not seem to be a JS Protobuf message.";
    return false;
  }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  return d.jsValueToMessage(
      engine_, *QJSValuePrivate::getValue(&raw)->as<ArrayObject>(), message);
#else
  Scope scope(engine_);
  ScopedArrayObject arr(scope, QJSValuePrivate::getValue(&raw));

  return d.jsValueToMessage(engine_, *arr, message);
#endif
}
}
}
