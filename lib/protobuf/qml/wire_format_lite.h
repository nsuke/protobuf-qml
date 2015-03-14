#ifndef PROTOBUF_QML_WIRE_FORMAT_LITE_H
#define PROTOBUF_QML_WIRE_FORMAT_LITE_H

#include "protobuf/qml/common.h"
#include "protobuf/qml/io.h"
#include <QObject>

namespace protobuf {
namespace qml {

class InputDevice;
class OutputDevice;

class PROTOBUF_QML_DLLEXPORT QmlWireFormatLite : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString error READ error)
  Q_PROPERTY(bool hasError READ has_error)

 public:
  QmlWireFormatLite(QObject* p = 0) : QObject(p) {}

  bool has_error() const { return has_error_; }

//  Q_INVOKABLE qint32 readTag(InputDevice*);
  Q_INVOKABLE qint32 readInt32(InputDevice*);
  Q_INVOKABLE void writeInt32(OutputDevice*, int tag, qint32 value);
  const QString& error() const;

 private:
  void clearError();
  void set_error(QString);

  bool has_error_ = false;
  QString last_error_;
  QString empty_error_;
};
}
}

#endif  // PROTOBUF_QML_WIRE_FORMAT_LITE_H
