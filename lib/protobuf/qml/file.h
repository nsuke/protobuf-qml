#ifndef PROTOBUF_QML_FILE_H
#define PROTOBUF_QML_FILE_H

#include "protobuf/qml/processor.h"
#include "protobuf/qml/common.h"
#include <QObject>

namespace protobuf {
namespace qml {

class PROTOBUF_QML_DLLEXPORT FileChannel : public BufferChannel {
  Q_OBJECT
  Q_PROPERTY(QString path READ path WRITE set_path NOTIFY pathChanged)

signals:
  void pathChanged();

public:
  explicit FileChannel(QObject* p = nullptr) : BufferChannel(p) {}

  const QString& path() { return path_; }
  void set_path(const QString& path) {
    if (path_ != path) {
      path_ = path;
      pathChanged();
    }
  }

  Q_INVOKABLE bool exists();
  Q_INVOKABLE void clear();

protected:
  google::protobuf::io::ZeroCopyInputStream* openInput(int tag) override;

  void closeInput(int tag,
                  google::protobuf::io::ZeroCopyInputStream* stream) override;

  google::protobuf::io::ZeroCopyOutputStream* openOutput(int tag,
                                                         int hint) override;

  void closeOutput(int tag,
                   google::protobuf::io::ZeroCopyOutputStream* stream) override;

private:
  const char* cPath() const { return path_.toStdString().c_str(); }

  QString path_;
  int file_;
};
}
}

#endif  // PROTOBUF_QML_FILE_H
