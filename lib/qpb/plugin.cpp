#include "qpb/plugin.h"
#include "qpb/descriptor_database.h"
#include "qpb/io.h"
#include "qpb/memory.h"
#include "qpb/wire_format_lite.h"
#include <QObject>
#include <QtQml>

QObject* wireFormatLiteFactory(QQmlEngine*, QJSEngine*) {
  return new qpb::QmlWireFormatLite;
}

QObject* descriptorPoolFactory(QQmlEngine*, QJSEngine*) {
  return new qpb::DescriptorPoolWrapper;
}

void ProtobufQmlTestPlugin::registerTypes(const char* uri) {
  qmlRegisterSingletonType<qpb::QmlWireFormatLite>(
      uri, 1, 0, "WireFormatLite", wireFormatLiteFactory);
  qmlRegisterSingletonType<qpb::DescriptorPoolWrapper>(
      uri, 1, 0, "DescriptorPool", descriptorPoolFactory);
  qmlRegisterUncreatableType<qpb::FileDescriptorWrapper>(uri, 1, 0, "FileDescriptor", "");
  qmlRegisterUncreatableType<qpb::DescriptorWrapper>(uri, 1, 0, "Descriptor", "");
  qmlRegisterType<qpb::InputDevice>(uri, 1, 0, "InputDevice");
  qmlRegisterType<qpb::OutputDevice>(uri, 1, 0, "OutputDevice");
  qmlRegisterType<qpb::MemoryBuffer>(uri, 1, 0, "MemoryBuffer");
}
