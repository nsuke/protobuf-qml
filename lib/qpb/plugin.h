#ifndef QPB_PLUGIN_H
#define QPB_PLUGIN_H

#include "qpb/common.h"
#include <QQmlExtensionPlugin>

class QPB_DLLEXPORT ProtobufQmlTestPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
 public:
  virtual void registerTypes(const char* uri) override;
};

#endif  // QPB_PLUGIN_H
