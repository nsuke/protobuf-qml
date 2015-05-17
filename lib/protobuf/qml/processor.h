#ifndef PROTOBUF_QML_PROCESSOR_H
#define PROTOBUF_QML_PROCESSOR_H

#include "protobuf/qml/descriptors.h"

#include <QDebug>
#include <QMetaType>
#include <QObject>

namespace google {
namespace protobuf {
namespace io {
class ZeroCopyInputStream;
class ZeroCopyOutputStream;
}
}
}

namespace protobuf {
namespace qml {

class Call : public QObject {
  Q_OBJECT

public:
  enum CallType {
    Unary = 0,
    Reader,
    Writer,
    ReaderWriter,
  };

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
  Q_ENUM(CallType)
#else
  Q_ENUMS(CallType)
#endif

private:
  Call() {}
};

class Processor;

class Channel : public QObject {
  Q_OBJECT

public:
  explicit Channel(QObject* p = nullptr) : QObject(p) {}
};

namespace detail {

class Worker : public QObject {
  Q_OBJECT

signals:
  void write(int tag, const QVariant& data);
  void writeEmpty(int tag);
  void writeEnd(int tag);

public:
  explicit Worker(Processor* p);

  ~Worker() {
    thread_.quit();
    thread_.wait();
  }

private:
  QThread thread_;
};
}

class Processor : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool async READ async WRITE set_async NOTIFY asyncChanged)
  Q_PROPERTY(protobuf::qml::Channel* channel READ channel WRITE set_channel
                 NOTIFY channelChanged)
  Q_PROPERTY(QString methodName READ method_name WRITE set_method_name NOTIFY
                 methodNameChanged)
  Q_PROPERTY(protobuf::qml::DescriptorWrapper* readDescriptor READ
                 read_descriptor WRITE set_read_descriptor NOTIFY
                     readDescriptorChanged)
  Q_PROPERTY(protobuf::qml::DescriptorWrapper* writeDescriptor READ
                 write_descriptor WRITE set_write_descriptor NOTIFY
                     writeDescriptorChanged)

signals:
  void asyncChanged();
  void channelChanged();
  void methodNameChanged();
  void readDescriptorChanged();
  void writeDescriptorChanged();
  void data(int tag, const QVariant& data);
  void dataEnd(int tag);
  void error(int tag, const QVariant& data);

public:
  explicit Processor(QObject* p = nullptr) : QObject(p) {}
  virtual ~Processor() {}

  Q_INVOKABLE virtual bool init() { return true; }

  bool async() const { return async_; }
  void set_async(bool async) {
    if (async != async_) {
      async_ = async;
      asyncChanged();
    }
  }

  Channel* channel() const { return channel_; }
  void set_channel(Channel* channel) {
    if (channel_ != channel) {
      stopWorker();
      channel_ = channel;
      channelChanged();
    }
  }

  const QString& method_name() const { return method_name_; }
  void set_method_name(const QString& method_name) {
    if (method_name_ != method_name) {
      stopWorker();
      method_name_ = method_name;
      methodNameChanged();
    }
  }

  DescriptorWrapper* read_descriptor() const { return read_desc_; }
  void set_read_descriptor(DescriptorWrapper* read_desc) {
    if (read_desc != read_desc_) {
      // Need to stop worker before switching resource accessed by worker
      // thread. This means that setting descriptor in middle of processing may
      // result in data loss.
      stopWorker();
      read_desc_ = read_desc;
      readDescriptorChanged();
    }
  }

  DescriptorWrapper* write_descriptor() const { return write_desc_; }
  void set_write_descriptor(DescriptorWrapper* write_desc) {
    if (write_desc != write_desc_) {
      stopWorker();
      write_desc_ = write_desc;
      writeDescriptorChanged();
    }
  }

  // returns seemingly unused tag, not guaranteed.
  Q_INVOKABLE int getFreeTag() { return ++max_tag_; }

  // TODO: need timeout
  Q_INVOKABLE void write(int tag, const QVariant& data);

  Q_INVOKABLE void writeEnd(int tag) {
    max_tag_ = std::max(tag, max_tag_);
    if (async_) {
      ensureWorker();
      worker_->writeEnd(tag);
    } else {
      writeEndThread(tag);
    }
  }

protected:
  virtual void doWrite(int tag, const google::protobuf::Message& msg) {
    error(tag, "Not supported.");
  }
  virtual void doEmptyWrite(int tag) {
    error(tag, "Cannot write empty message");
  }

  virtual void doWriteEnd(int tag) { error(tag, "Not supported."); }

  void emitMessageData(int tag, const google::protobuf::Message& msg) {
    if (!read_desc_) {
      qWarning() << "Descriptor is null.";
      // TODO: need to emit error here ?
      return;
    }
    data(tag, read_desc_->dataFromMessage(msg));
  }

private:
  void stopWorker() { worker_.reset(); }

  void ensureWorker() {
    if (!worker_) {
      worker_.reset(new detail::Worker(this));
    }
  }

  void writeThread(int tag, const QVariant& data);

  void writeEmptyThread(int tag) { doEmptyWrite(tag); }

  void writeEndThread(int tag) {
    if (!write_desc_) {
      qWarning("Descriptor is null");
      error(tag, "Descriptor is null.");
      return;
    }
    doWriteEnd(tag);
  }

  bool async_ = true;
  int max_tag_ = 0;
  Channel* channel_;
  QString method_name_;
  DescriptorWrapper* read_desc_ = nullptr;
  DescriptorWrapper* write_desc_ = nullptr;

  std::unique_ptr<detail::Worker> worker_;
  friend class detail::Worker;
};

class BufferChannel;

class BufferMethod : public Processor {
  Q_OBJECT

public:
  explicit BufferMethod(QObject* p = nullptr) : Processor(p) {
    connect(this, &Processor::channelChanged, [this] {
      if (channel() && !(buffer_ = qobject_cast<BufferChannel*>(channel()))) {
        qWarning() << "Incompatible channel.";
      }
    });
  }

protected:
  void doWrite(int tag, const google::protobuf::Message& msg) final;
  void doEmptyWrite(int tag) final;

private:
  BufferChannel* buffer_;
};

class BufferChannel : public Channel {
  Q_OBJECT

signals:
  void parseError(int tag, const QVariant& data);
  void serializeError(int tag, const QVariant& data);

public:
  explicit BufferChannel(QObject* p = nullptr) : Channel(p) {}

protected:
  virtual google::protobuf::io::ZeroCopyInputStream* openInput(int tag) {
    return nullptr;
  }
  virtual google::protobuf::io::ZeroCopyOutputStream* openOutput(int tag,
                                                                 int hint) {
    return nullptr;
  }
  virtual void closeOutput(int tag,
                           google::protobuf::io::ZeroCopyOutputStream* stream) {
  }
  virtual void closeInput(int tag,
                          google::protobuf::io::ZeroCopyInputStream* stream) {}

private:
  friend class BufferMethod;
};
}
}

#endif  // PROTOBUF_QML_PROCESSOR_H
