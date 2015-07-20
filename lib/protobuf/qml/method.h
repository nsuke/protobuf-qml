#ifndef PROTOBUF_QML_METHOD_H
#define PROTOBUF_QML_METHOD_H

#include "protobuf/qml/common.h"
#include "protobuf/qml/descriptors.h"
#include <QObject>

#include <memory>
Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(std::shared_ptr<google::protobuf::Message>)

namespace protobuf {
namespace qml {

class PROTOBUF_QML_DLLEXPORT StatusCode {
  Q_GADGET
  Q_ENUMS(StatusCode_)

public:
  // Synced with grpc++/status_code_enum.h
  // vim search phrase:
  // ^\s*[A-Z_]\+\s*=\s*-\?[0-9]\+,\?$
  enum StatusCode_ {
    CANCELLED = 1,
    UNKNOWN = 2,
    INVALID_ARGUMENT = 3,
    DEADLINE_EXCEEDED = 4,
    NOT_FOUND = 5,
    ALREADY_EXISTS = 6,
    PERMISSION_DENIED = 7,
    UNAUTHENTICATED = 16,
    RESOURCE_EXHAUSTED = 8,
    FAILED_PRECONDITION = 9,
    ABORTED = 10,
    OUT_OF_RANGE = 11,
    UNIMPLEMENTED = 12,
    INTERNAL = 13,
    UNAVAILABLE = 14,
    DATA_LOSS = 15,
    DO_NOT_USE = -1
  };
};

class Channel2;

class PROTOBUF_QML_DLLEXPORT MethodBase : public QObject {
  Q_OBJECT

signals:
  void data(int tag, const std::shared_ptr<google::protobuf::Message>& data);
  void error(int tag, int code, const QString& message);
  void closed(int tag);

public:
  void unknownError(int tag, const QString& message) {
    error(tag, StatusCode::UNKNOWN, message);
  }

protected:
  explicit MethodBase(QObject* p = nullptr) : QObject(p) {
    qRegisterMetaType<std::shared_ptr<google::protobuf::Message>>();
  }
  virtual ~MethodBase() {}
};

class PROTOBUF_QML_DLLEXPORT MethodHolder : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString methodName READ method_name WRITE set_method_name NOTIFY
                 methodNameChanged)

  Q_PROPERTY(protobuf::qml::DescriptorWrapper* readDescriptor READ
                 read_descriptor WRITE set_read_descriptor NOTIFY
                     readDescriptorChanged)
  Q_PROPERTY(protobuf::qml::DescriptorWrapper* writeDescriptor READ
                 write_descriptor WRITE set_write_descriptor NOTIFY
                     writeDescriptorChanged)

signals:
  void data(int tag, const QJSValue& data);
  void error(int tag, int code, const QString& message);
  void closed(int tag);

  void methodNameChanged();
  void readDescriptorChanged();
  void writeDescriptorChanged();
  // void implChanged();

public:
  explicit MethodHolder(QObject* p = nullptr) : QObject(p) {}
  virtual ~MethodHolder() {}

  const QString& method_name() const { return method_name_; }
  void set_method_name(const QString& method_name) {
    if (method_name_ != method_name) {
      deinit();
      method_name_ = method_name;
      methodNameChanged();
    }
  }

  DescriptorWrapper* read_descriptor() const { return read_desc_; }
  void set_read_descriptor(DescriptorWrapper* read_desc);

  DescriptorWrapper* write_descriptor() const { return write_desc_; }
  void set_write_descriptor(DescriptorWrapper* write_desc);

public slots:
  void handleData(int tag,
                  const std::shared_ptr<google::protobuf::Message>& data);

protected:
  virtual void deinit() {}

  // private:
  QV4::ExecutionEngine* v4 = nullptr;
  QString method_name_;
  DescriptorWrapper* read_desc_ = nullptr;
  DescriptorWrapper* write_desc_ = nullptr;
};

class PROTOBUF_QML_DLLEXPORT ClientMethodHolder : public MethodHolder {
  Q_OBJECT

  Q_PROPERTY(protobuf::qml::Channel2* channel READ channel WRITE set_channel
                 NOTIFY channelChanged)

signals:
  void channelChanged();

public:
  explicit ClientMethodHolder(QObject* p = nullptr) : MethodHolder(p) {}
  virtual ~ClientMethodHolder() {}

  Channel2* channel() const { return channel_; }
  void set_channel(Channel2* channel) {
    if (channel != channel_) {
      deinit();
      channel_ = channel;
      channelChanged();
    }
  }

protected:
  bool readyForInit() {
    return channel_ && !method_name_.isEmpty() && read_desc_ && write_desc_;
  }

private:
  Channel2* channel_ = nullptr;
};

class PROTOBUF_QML_DLLEXPORT UnaryMethod : public MethodBase {
  Q_OBJECT

public:
  explicit UnaryMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~UnaryMethod() {}
  virtual bool write(int tag,
                     std::unique_ptr<google::protobuf::Message>,
                     int timeout) {
    return false;
  }
};

class PROTOBUF_QML_DLLEXPORT UnaryMethodHolder : public ClientMethodHolder {
  Q_OBJECT

public:
  explicit UnaryMethodHolder(QObject* p = nullptr) : ClientMethodHolder(p) {}
  ~UnaryMethodHolder() {}

  Q_INVOKABLE void write(QQmlV4Function*);

protected:
  void deinit() final {
    // TODO: Is it safe at any time ??
    impl_.reset();
  }

private:
  bool ensureInit();

  std::unique_ptr<UnaryMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT WriterMethod : public MethodBase {
  Q_OBJECT

public:
  explicit WriterMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~WriterMethod() {}
  virtual bool write(int tag, const QVariant& data) { return false; }
  virtual bool writesDone(int tag) { return false; }
  virtual int timeout(int tag) const { return -1; }
  virtual void set_timeout(int tag, int milliseconds) {}
};

class PROTOBUF_QML_DLLEXPORT WriterMethodHolder : public ClientMethodHolder {
  Q_OBJECT

public:
  explicit WriterMethodHolder(QObject* p = nullptr) : ClientMethodHolder(p) {}
  ~WriterMethodHolder() {}

  Q_INVOKABLE bool write(int tag, const QVariant& data) {
    if (!ensureInit()) {
      return false;
    }
    return impl_->write(tag, data);
  }

  Q_INVOKABLE bool writesDone(int tag) {
    if (!ensureInit()) {
      return false;
    }
    return impl_->writesDone(tag);
  }

  Q_INVOKABLE int timeout(int tag) {
    if (!ensureInit()) {
      return -1;
    }
    return impl_->timeout(tag);
  }
  Q_INVOKABLE void set_timeout(int tag, int milliseconds) {
    if (!ensureInit()) {
      return;
    }
    impl_->set_timeout(tag, milliseconds);
  }

protected:
  void deinit() final {
    // TODO: Is it safe at any time ??
    impl_.reset();
  }

private:
  bool ensureInit();

  std::unique_ptr<WriterMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT ReaderMethod : public MethodBase {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ReaderMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~ReaderMethod() {}

  virtual bool write(int tag, const QVariant& data, int timeout) {
    return false;
  }
};

class PROTOBUF_QML_DLLEXPORT ReaderMethodHolder : public ClientMethodHolder {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ReaderMethodHolder(QObject* p = nullptr) : ClientMethodHolder(p) {}
  ~ReaderMethodHolder() {}

  Q_INVOKABLE bool write(int tag, const QVariant& data, int timeout) {
    if (!ensureInit()) {
      return false;
    }
    return impl_->write(tag, data, timeout);
  }

protected:
  void deinit() final {
    // TODO: Is it safe at any time ??
    impl_.reset();
  }

private:
  bool ensureInit();

  std::unique_ptr<ReaderMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT ReaderWriterMethod : public MethodBase {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ReaderWriterMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~ReaderWriterMethod() {}
  virtual bool call(int tag) { return false; }
  virtual bool write(int tag, const QVariant& data) { return false; }
  virtual bool writesDone(int tag) { return false; }
  virtual int timeout(int tag) const { return -1; }
  virtual void set_timeout(int tag, int milliseconds) {}
};

class PROTOBUF_QML_DLLEXPORT ReaderWriterMethodHolder
    : public ClientMethodHolder {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ReaderWriterMethodHolder(QObject* p = nullptr)
      : ClientMethodHolder(p) {}
  ~ReaderWriterMethodHolder() {}

  Q_INVOKABLE bool call(int tag) {
    if (!ensureInit()) {
      return false;
    }
    return impl_->call(tag);
  }

  Q_INVOKABLE bool write(int tag, const QVariant& data) {
    if (!ensureInit()) {
      return false;
    }
    return impl_->write(tag, data);
  }

  Q_INVOKABLE bool writesDone(int tag) {
    if (!ensureInit()) {
      return false;
    }
    return impl_->writesDone(tag);
  }

  Q_INVOKABLE int timeout(int tag) {
    if (!ensureInit()) {
      return -1;
    }
    return impl_->timeout(tag);
  }
  Q_INVOKABLE void set_timeout(int tag, int milliseconds) {
    if (!ensureInit()) {
      return;
    }
    impl_->set_timeout(tag, milliseconds);
  }

protected:
  void deinit() final { impl_.reset(); }

private:
  bool ensureInit();

  std::unique_ptr<ReaderWriterMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT Channel2 : public QObject {
  Q_OBJECT

public:
  explicit Channel2(QObject* p = nullptr) : QObject(p) {}

  virtual ~Channel2() {}

  virtual UnaryMethod* registerUnaryMethod(const QString& name,
                                           DescriptorWrapper* read,
                                           DescriptorWrapper* write) {
    return nullptr;
  }

  virtual ReaderMethod* registerReaderMethod(const QString& name,
                                             DescriptorWrapper* read,
                                             DescriptorWrapper* write) {
    return nullptr;
  }

  virtual WriterMethod* registerWriterMethod(const QString& name,
                                             DescriptorWrapper* read,
                                             DescriptorWrapper* write) {
    return nullptr;
  }

  virtual ReaderWriterMethod* registerReaderWriterMethod(
      const QString& name, DescriptorWrapper* read, DescriptorWrapper* write) {
    return nullptr;
  }
};
}
}
#endif  // PROTOBUF_QML_METHOD_H
