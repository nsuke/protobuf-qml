#ifndef PROTOBUF_QML_FILE_H
#define PROTOBUF_QML_FILE_H

#include "protobuf/qml/common.h"
#include <QObject>

namespace protobuf {
namespace qml {

class PROTOBUF_QML_DLLEXPORT InputContext : public QObject {
  Q_OBJECT
 public:
  InputContext(QObject* p = nullptr) : QObject(p) {}

  Q_INVOKABLE bool parseTo(QVariantMap value) {}
};

class PROTOBUF_QML_DLLEXPORT FileInput : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString path READ path WRITE set_path NOTIFY pathChanged)

 signals:
  void pathChanged();

 public:
  FileInput(QObject* p = nullptr) : QObject(p) {}

  const QString& path() { return path_; }
  void set_path(const QString& path) {
    if (path_ != path) {
      path_ = path;
      pathChanged();
    }
  }

  Q_INVOKABLE InputContext* createContext() {}

 private:
  QString path_;
};
}
}

#endif  // PROTOBUF_QML_FILE_H
