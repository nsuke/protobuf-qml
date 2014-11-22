#ifndef QPB_WIRE_FORMAT_LITE_H
#define QPB_WIRE_FORMAT_LITE_H

#include "qpb/common.h"
#include "qpb/io.h"
#include <QObject>

namespace qpb {

class InputDevice;
class OutputDevice;

class QPB_DLLEXPORT QmlWireFormatLite : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString error READ error)
  Q_PROPERTY(bool hasError READ has_error)

 public:
  QmlWireFormatLite(QObject* p = 0) : QObject(p) {}

  bool has_error() const { return has_error_; }

  Q_INVOKABLE qint32 readTag(InputDevice*);
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

#endif  // QPB_WIRE_FORMAT_LITE_H
