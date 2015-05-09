#ifndef GRPC_QML_GLOBAL_INSTANCE_H
#define GRPC_QML_GLOBAL_INSTANCE_H

#include <grpc/grpc.h>
#include <QObject>
#include "protobuf/qml/common.h"

namespace grpc {
namespace qml {

class GRPC_QML_DLLEXPORT GlobalInstance : public QObject {
  Q_OBJECT
public:
  GlobalInstance(QObject* p = 0) : QObject(p) {}
  ~GlobalInstance();
  Q_INVOKABLE void init() { grpc_init(); }
  Q_INVOKABLE void shutdown() { grpc_shutdown(); }
};
}
}
#endif  // GRPC_QML_GLOBAL_INSTANCE_H
