#ifndef GRPC_QML_GRPC_PLUGIN_H
#define GRPC_QML_GRPC_PLUGIN_H

#include <QQmlExtensionPlugin>

class GrpcQmlPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
  virtual void registerTypes(const char* uri) override;
};

#endif  // GRPC_QML_GRPC_PLUGIN_H
