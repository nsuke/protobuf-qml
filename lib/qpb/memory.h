#ifndef QPB_MEMORY_H
#define QPB_MEMORY_H

#include "qpb/io.h"
#include "qpb/common.h"

#include <QObject>
#include <QVariantMap>
#include <memory>

namespace google {
namespace protobuf {
namespace io {
class ArrayInputStream;
class ArrayOutputStream;
}
}
}

namespace qpb {

namespace detail {

template <typename Stream, typename StreamImpl>
class MemoryIOImpl {
 public:
  MemoryIOImpl(std::vector<char>* buffer) : buffer_(buffer) {}

  StreamImpl* createStream() {
    if (in_use_)
      return nullptr;
    in_use_ = true;
    auto bsize = block_size_ >= 1 ? block_size_ : -1;
    return new StreamImpl(buffer_->data(), buffer_->size(), bsize);
  }

  void notify() { in_use_ = false; }

  void set_block_size(int size) { block_size_ = size; }

 private:
  int block_size_ = 0;
  bool in_use_ = false;
  std::vector<char>* buffer_;
};
}

class MemoryInput : public InputDevice {
  Q_OBJECT
 public:
  MemoryInput(std::vector<char>* buffer, QObject* p = nullptr)
      : InputDevice(p), impl_(buffer) {
    QPB_LOG_F;
  }

  virtual SessionPtr createSession() override;

  virtual void notify() override { impl_.notify(); }

  void set_block_size(int size) { impl_.set_block_size(size); }

 private:
  detail::MemoryIOImpl<google::protobuf::io::ZeroCopyInputStream,
                       google::protobuf::io::ArrayInputStream> impl_;
};

class MemoryOutput : public OutputDevice {
  Q_OBJECT
 public:
  MemoryOutput(std::vector<char>* buffer, QObject* p = nullptr)
      : OutputDevice(p), impl_(buffer) {}

  virtual SessionPtr createSession() override;

  virtual void notify() override { impl_.notify(); }

  void set_block_size(int size) { impl_.set_block_size(size); }

 private:
  detail::MemoryIOImpl<google::protobuf::io::ZeroCopyOutputStream,
                       google::protobuf::io::ArrayOutputStream> impl_;
};

class QPB_DLLEXPORT MemoryBuffer : public QObject {
  Q_OBJECT
  Q_PROPERTY(int size READ size WRITE set_size NOTIFY sizeChanged)
  Q_PROPERTY(int blockSize READ block_size WRITE set_block_size NOTIFY
                 blockSizeChanged)
  Q_PROPERTY(qpb::InputDevice* input READ input)
  Q_PROPERTY(qpb::OutputDevice* output READ output)

signals:
  void sizeChanged();
  void blockSizeChanged();

 public:
  MemoryBuffer(QObject* p = nullptr)
      : QObject(p), input_(&buffer_), output_(&buffer_) {}

  int block_size() const { return block_size_; }
  void set_block_size(int size);

  int size() const { return size_; }
  void set_size(int size);

  Q_INVOKABLE void clear() {
    buffer_.clear();
    buffer_.resize(size_);
  }

  InputDevice* input() { return &input_; }

  OutputDevice* output() { return &output_; }

 private:
  int size_ = 0;
  int block_size_ = 0;
  std::vector<char> buffer_;
  MemoryInput input_;
  MemoryOutput output_;
};
}

#endif  // QPB_MEMORY_H
