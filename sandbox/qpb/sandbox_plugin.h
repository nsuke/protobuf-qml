#ifndef QPB_PLUGIN_H
#define QPB_PLUGIN_H

#include <sys/cdefs.h>
#ifndef __extern_always_inline
#define __extern_always_inline inline
#endif
#include <QQmlExtensionPlugin>

class Q_DECL_EXPORT ProtobufQmlPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
 public:
  virtual void registerTypes(const char* uri) override;
};

#endif  // QPB_PLUGIN_H
