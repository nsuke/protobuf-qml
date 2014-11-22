#include "qpb/sandbox_plugin.h"
#include "qpb/sandbox.h"
#include <QtQml>

QObject* sandboxFactory(QQmlEngine*, QJSEngine*) {
  return new qpb::Sandbox;
}

void ProtobufQmlPlugin::registerTypes(const char* uri) {
  qmlRegisterSingletonType<qpb::Sandbox>(
      uri, 1, 0, "DefaultSandbox", sandboxFactory);
  qmlRegisterType<qpb::Sandbox>(uri, 1, 0, "Sandbox");
}
