#include "protobuf/qml/plugin.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/io.h"
#include "protobuf/qml/memory.h"
#include "protobuf/qml/wire_format_lite.h"
#include "protobuf/qml/processor.h"
#include <QObject>
#include <QtQml>

QObject* wireFormatLiteFactory(QQmlEngine*, QJSEngine*) {
  return new protobuf::qml::QmlWireFormatLite;
}

QObject* descriptorPoolFactory(QQmlEngine*, QJSEngine*) {
  return new protobuf::qml::DescriptorPoolWrapper;
}

void ProtobufQmlTestPlugin::registerTypes(const char* uri) {
  qmlRegisterSingletonType<protobuf::qml::QmlWireFormatLite>(
      uri, 1, 0, "WireFormatLite", wireFormatLiteFactory);
  qmlRegisterSingletonType<protobuf::qml::DescriptorPoolWrapper>(
      uri, 1, 0, "DescriptorPool", descriptorPoolFactory);
  qmlRegisterUncreatableType<protobuf::qml::FileDescriptorWrapper>(
      uri, 1, 0, "FileDescriptor", "");
  qmlRegisterUncreatableType<protobuf::qml::DescriptorWrapper>(
      uri, 1, 0, "Descriptor", "");
  qmlRegisterType<protobuf::qml::AsyncProcessor>(uri, 1, 0, "AsyncProcessor");
  qmlRegisterType<protobuf::qml::InputDevice>(uri, 1, 0, "InputDevice");
  qmlRegisterType<protobuf::qml::OutputDevice>(uri, 1, 0, "OutputDevice");
  qmlRegisterType<protobuf::qml::MemoryBuffer>(uri, 1, 0, "MemoryBuffer");
}
