#include "grpc/qml/grpc_plugin.h"
#include "grpc/qml/channel.h"
#include "grpc/qml/credentials.h"
#include "grpc/qml/global_instance.h"

#include <QObject>
#include <QtQml>

QObject* globalInstanceFactory(QQmlEngine*, QJSEngine*) {
  return new grpc::qml::GlobalInstance;
}

void GrpcQmlPlugin::registerTypes(const char* uri) {
  qmlRegisterType<grpc::qml::Channel>(uri, 1, 0, "Channel");
  qmlRegisterType<grpc::qml::InsecureCredentials>(uri, 1, 0, "InsecureCredentials");
  qmlRegisterUncreatableType<grpc::qml::Credentials>(uri, 1, 0, "Credentials", "Credentials is base type that cannot be instantiated itself.");
  qmlRegisterSingletonType<grpc::qml::GlobalInstance>(uri, 1, 0, "Instance", globalInstanceFactory);
}

