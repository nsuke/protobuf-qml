#ifndef GRPC_QML_GRPC_PLUGIN_H
#define GRPC_QML_GRPC_PLUGIN_H

#include "protobuf/qml/common.h"

#include <QQmlExtensionPlugin>

class GRPC_QML_DLLEXPORT GrpcQmlPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
  virtual void registerTypes(const char* uri) override;
};

#endif  // GRPC_QML_GRPC_PLUGIN_H
