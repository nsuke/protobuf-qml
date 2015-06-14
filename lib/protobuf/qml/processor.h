#ifndef PROTOBUF_QML_PROCESSOR_H
#define PROTOBUF_QML_PROCESSOR_H

#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/common.h"

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

class Method;

class Channel : public QObject {
  Q_OBJECT

public:
  explicit Channel(QObject* p = nullptr) : QObject(p) {}
  virtual ~Channel() {}

  virtual bool registerMethod(Method* method, int call_type) { return true; }
  virtual void deregisterMethod(Method* method) {}

  virtual void write(Method* method,
                     int tag,
                     const google::protobuf::Message& msg) {}
  virtual void writeEmpty(Method* method, int tag) {}
  virtual void writeEnd(Method* method, int tag) {}

protected:
  void emitMessageData(Method* method,
                       int tag,
                       const google::protobuf::Message& msg);
};

namespace detail {

class Worker : public QObject {
  Q_OBJECT

signals:
  void write(int tag, const QVariant& data);
  void writeEmpty(int tag);
  void writeEnd(int tag);

public:
  explicit Worker(Method* p);

  ~Worker() {
    thread_.quit();
    thread_.wait();
  }

private:
  QThread thread_;
};
}

class Method : public QObject {
  Q_OBJECT

  Q_PROPERTY(int callType READ call_type WRITE set_call_type NOTIFY
                 callTypeChanged)
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

signals:
  void callTypeChanged();
  void asyncChanged();
  void channelChanged();
  void methodNameChanged();
  void readDescriptorChanged();
  void writeDescriptorChanged();
  void data(int tag, const QVariant& data);
  void dataEnd(int tag);
  void error(int tag, const QString& data);

public:
  explicit Method(QObject* p = nullptr) : QObject(p) {}
  virtual ~Method() { deinit(); }

  CallType call_type() const { return call_type_; }
  void set_call_type(int call_type) {
    if (call_type_ != call_type) {
      call_type_ = static_cast<CallType>(call_type);
      callTypeChanged();
    }
  }

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
      deinit();
      channel_ = channel;
      init();
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

  Q_INVOKABLE void writeEnd(int tag);

protected:
  void emitMessageData(int tag, const google::protobuf::Message& msg) {
    if (!read_desc_) {
      qWarning() << "Descriptor is null.";
      // TODO: need to emit error here ?
      return;
    }
    data(tag, read_desc_->dataFromMessage(msg));
  }

private:
  bool init() {
    if (!initialized_ && channel_) {
      initialized_ = channel_->registerMethod(this, call_type_);
    }
    return initialized_;
  }

  void deinit() {
    if (channel_) {
      channel_->deregisterMethod(this);
      initialized_ = false;
    }
  }

  void stopWorker() { worker_.reset(); }

  void ensureWorker() {
    if (!worker_) {
      worker_.reset(new detail::Worker(this));
    }
  }

  void writeThread(int tag, const QVariant& data);

  void writeEmptyThread(int tag) { channel_->writeEmpty(this, tag); }

  void writeEndThread(int tag) { channel_->writeEnd(this, tag); }

  // Interface to channel
  void emitData(int tag, const QVariant& data) {
    auto msg = write_descriptor()->dataToMessage(data);
    emitMessageData(tag, *msg);
  }

  bool initialized_ = false;
  CallType call_type_;
  bool async_ = true;
  int max_tag_ = 0;
  Channel* channel_ = nullptr;
  QString method_name_;
  DescriptorWrapper* read_desc_ = nullptr;
  DescriptorWrapper* write_desc_ = nullptr;

  std::unique_ptr<detail::Worker> worker_;
  friend class detail::Worker;
  friend class Channel;
};

class PROTOBUF_QML_DLLEXPORT BufferChannel : public Channel {
  Q_OBJECT

public:
  explicit BufferChannel(QObject* p = nullptr) : Channel(p) {}
  virtual ~BufferChannel() {}

  void write(Method* method,
             int tag,
             const google::protobuf::Message& msg) override;
  void writeEmpty(Method* method, int tag) override;

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
};
}
}

#endif  // PROTOBUF_QML_PROCESSOR_H
