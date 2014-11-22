#ifndef QPB_FILE_H
#define QPB_FILE_H

#include "qpb/common.h"
#include <QObject>

namespace qpb {

class QPB_DLLEXPORT InputContext : public QObject {
  Q_OBJECT
 public:
  InputContext(QObject* p = nullptr) : QObject(p) {}

  Q_INVOKABLE bool parseTo(QVariantMap value) {}
};

class QPB_DLLEXPORT FileInput : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString path READ path WRITE set_path NOTIFY pathChanged)

signals:
  void pathChanged();

 public:
  FileInput(QObject* p = nullptr) : QObject(p) {}

  Q_INVOKABLE InputContext* createContext() {}

 private:
  QString path_;
};
}

#endif  // QPB_FILE_H
