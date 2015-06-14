#include "protobuf/qml/protobuf_plugin.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/memory.h"
#include "protobuf/qml/file.h"
#include "protobuf/qml/processor.h"
#include "protobuf/qml/method.h"
#include <QObject>
#include <QtQml>

QObject* descriptorPoolFactory(QQmlEngine*, QJSEngine*) {
  return new protobuf::qml::DescriptorPoolWrapper;
}

void ProtobufQmlPlugin::registerTypes(const char* uri) {
  qmlRegisterSingletonType<protobuf::qml::DescriptorPoolWrapper>(
      uri, 1, 0, "DescriptorPool", descriptorPoolFactory);
  qmlRegisterUncreatableType<protobuf::qml::FileDescriptorWrapper>(
      uri, 1, 0, "FileDescriptor", "");
  qmlRegisterUncreatableType<protobuf::qml::DescriptorWrapper>(
      uri, 1, 0, "Descriptor", "");
  qmlRegisterType<protobuf::qml::Channel>(uri, 1, 0, "Channel");
  qmlRegisterType<protobuf::qml::Method>(uri, 1, 0, "Method");

  qmlRegisterType<protobuf::qml::BufferChannel>(uri, 1, 0, "BufferChannel");
  qmlRegisterType<protobuf::qml::MemoryBufferChannel>(uri, 1, 0, "MemoryBufferChannel");
  qmlRegisterType<protobuf::qml::FileChannel>(uri, 1, 0, "FileChannel");


  qmlRegisterType<protobuf::qml::Channel2>(uri, 1, 0, "Channel2");
  qmlRegisterType<protobuf::qml::UnaryMethodHolder>(uri, 1, 0, "UnaryMethodHolder");
}
