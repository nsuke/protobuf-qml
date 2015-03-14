#ifndef PROTOBUF_QML_IO2_H
#define PROTOBUF_QML_IO2_H

namespace protobuf {
namespace qml {
typedef ScopedSession<OutputDevice, google::protobuf::io::ZeroCopyOutputStream>
    SessionPtr;

SessionPtr createSession();

class PROTOBUF_QML_DLLEXPORT OutputDevice : public QObject {
  Q_OBJECT

signals:
  void dataWritten();

 public:
  OutputDevice(QObject* p = nullptr) : QObject(p) {}

  virtual ~OutputDevice() {}

  virtual void notify() {}
};
}
}
#endif  // PROTOBUF_QML_IO2_H
