#ifndef GRPC_QML_CHANNEL_H
#define GRPC_QML_CHANNEL_H

#include "grpc/qml/credentials.h"
#include <QObject>

namespace grpc {
namespace qml {

class Channel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString target READ target WRITE set_target NOTIFY targetChanged)
  Q_PROPERTY(Credentials* credentials READ credentials WRITE set_credentials
                 NOTIFY credentialsChanged)

signals:
  void credentialsChanged();
  void targetChanged();

public:
  const QString& target() const { return target_; }
  void set_target(const QString& target) {
    if (target != target_) {
      target_ = target;
      targetChanged();
    }
  }

  Channel(QObject* p = 0) : QObject(p) {}
  Credentials* credentials() const { return creds_; }
  void set_credentials(Credentials* creds) {
    if (creds != creds_) {
      creds_ = creds;
      credentialsChanged();
    }
  }

private:
  QString target_;
  Credentials* creds_;
};
}
}

#endif  // GRPC_QML_CHANNEL_H
