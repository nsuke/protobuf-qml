#include "grpc/qml/grpc_plugin.h"
#include "grpc/qml/credentials.h"
#include "grpc/qml/base.h"

#include <QObject>
#include <QtQml>

void GrpcQmlPlugin::registerTypes(const char* uri) {
  qmlRegisterType<grpc::qml::Channel>(uri, 1, 0, "GrpcChannel");

  qmlRegisterType<grpc::qml::InsecureCredentials>(uri, 1, 0,
                                                  "InsecureCredentials");
  qmlRegisterUncreatableType<grpc::qml::Credentials>(
      uri, 1, 0, "Credentials",
      "Credentials is base type that cannot be instantiated itself.");
}
