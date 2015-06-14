#ifndef PROTOBUF_QML_PROTOBUF_PLUGIN_H
#define PROTOBUF_QML_PROTOBUF_PLUGIN_H

#include "protobuf/qml/common.h"
#include <QQmlExtensionPlugin>

class ProtobufQmlPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
  virtual void registerTypes(const char* uri) override;
};

#endif  // PROTOBUF_QML_PROTOBUF_PLUGIN_H
