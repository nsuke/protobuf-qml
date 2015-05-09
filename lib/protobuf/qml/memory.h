#ifndef PROTOBUF_QML_MEMORY_H
#define PROTOBUF_QML_MEMORY_H

#include "protobuf/qml/processor.h"
#include "protobuf/qml/common.h"

#include <QObject>
#include <QVariantMap>
#include <memory>

namespace protobuf {
namespace qml {

// It doesn't manage offset at all.
class PROTOBUF_QML_DLLEXPORT MemoryBuffer : public GenericStreamProcessor {
  Q_OBJECT
  Q_PROPERTY(int size READ size WRITE set_size NOTIFY sizeChanged)
  Q_PROPERTY(int blockSize READ block_size WRITE set_block_size NOTIFY
                 blockSizeChanged)

signals:
  void sizeChanged();
  void blockSizeChanged();

 public:
  explicit MemoryBuffer(QObject* p = nullptr) : GenericStreamProcessor(p) {}

  int block_size() const { return block_size_; }
  void set_block_size(int size);

  int size() const { return size_; }
  void set_size(int size);

  Q_INVOKABLE void clear() {
    buffer_.clear();
    buffer_.resize(size_);
  }

 protected:
  google::protobuf::io::ZeroCopyInputStream* openInput(int tag) override;

  google::protobuf::io::ZeroCopyOutputStream* openOutput(int tag, int hint) override;

 private:
  int effective_block_size() const {
    return block_size_ > 0 ? block_size_ : -1;
  }

  int size_ = 0;
  int block_size_ = 0;
  std::vector<char> buffer_;
};
}
}

#endif  // PROTOBUF_QML_MEMORY_H
