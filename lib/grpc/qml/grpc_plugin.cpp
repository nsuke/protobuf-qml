#include "grpc/qml/grpc_plugin.h"
#include "grpc/qml/credentials.h"
#include "grpc/qml/server_credentials.h"
#include "grpc/qml/base.h"
#include "grpc/qml/server.h"

#include <QObject>
#include <QtQml>

void GrpcQmlPlugin::registerTypes(const char* uri) {
  qmlRegisterType<grpc::qml::Channel>(uri, 1, 0, "GrpcChannel");

  qmlRegisterType<grpc::qml::InsecureCredentials>(uri, 1, 0,
                                                  "InsecureCredentials");
  qmlRegisterUncreatableType<grpc::qml::Credentials>(
      uri, 1, 0, "Credentials",
      "Credentials is base type that cannot be instantiated itself.");

  qmlRegisterType<grpc::qml::GrpcServer>(uri, 1, 0, "GrpcServer");
  qmlRegisterType<grpc::qml::InsecureServerCredentials>(
      uri, 1, 0, "InsecureServerCredentials");
  qmlRegisterUncreatableType<grpc::qml::ServerCredentials>(
      uri, 1, 0, "ServerCredentials",
      "Credentials is base type that cannot be instantiated itself.");
}
