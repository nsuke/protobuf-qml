#ifndef PROTOBUF_QML_BUFFER_H
#define PROTOBUF_QML_BUFFER_H

#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/method.h"
#include "protobuf/qml/common.h"

namespace protobuf {
namespace qml {

class BufferChannel;

class PROTOBUF_QML_DLLEXPORT SerializeMethod : public UnaryMethod {
public:
  SerializeMethod(BufferChannel* channel, DescriptorWrapper* descriptor);
  bool write(int tag, std::unique_ptr<google::protobuf::Message>, int timeout) final;

private:
  BufferChannel* channel_;
  DescriptorWrapper* descriptor_;
};

class PROTOBUF_QML_DLLEXPORT ParseMethod : public UnaryMethod {
public:
  ParseMethod(BufferChannel* channel, DescriptorWrapper* descriptor);
  bool write(int tag, std::unique_ptr<google::protobuf::Message>, int timeout) final;

private:
  BufferChannel* channel_;
  DescriptorWrapper* descriptor_;
};

class PROTOBUF_QML_DLLEXPORT BufferChannel : public Channel2 {
  Q_OBJECT

public:
  BufferChannel(QObject* p = nullptr) : Channel2(p) {}
  virtual ~BufferChannel() {}

  UnaryMethod* registerUnaryMethod(const QString& name,
                                   DescriptorWrapper* read,
                                   DescriptorWrapper* write) final;
  virtual google::protobuf::io::ZeroCopyInputStream* openInput(int tag) = 0;
  virtual google::protobuf::io::ZeroCopyOutputStream* openOutput(int tag,
                                                                 int hint) = 0;
  virtual void closeOutput(int tag,
                           google::protobuf::io::ZeroCopyOutputStream* stream) {
  }
  virtual void closeInput(int tag,
                          google::protobuf::io::ZeroCopyInputStream* stream) {}
};
}
}
#endif  // PROTOBUF_QML_BUFFER_H
