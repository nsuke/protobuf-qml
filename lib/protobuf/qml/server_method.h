#ifndef PROTOBUF_QML_SERVER_METHOD_H
#define PROTOBUF_QML_SERVER_METHOD_H

#include "protobuf/qml/method.h"
#include "protobuf/qml/common.h"
#include "protobuf/qml/descriptors.h"
#include <QObject>
#include <QQuickItem>

namespace protobuf {
namespace qml {

class PROTOBUF_QML_DLLEXPORT ServerMethod : public MethodBase {
  Q_OBJECT

public:
  explicit ServerMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~ServerMethod() {}

  virtual bool respond(int tag, const QVariant& data) { return false; }

  virtual bool abort(int tag, int error_code, const QString& error_message) {
    return false;
  }

  virtual void startProcessing() {}
};

class PROTOBUF_QML_DLLEXPORT ServerMethodHolder : public MethodHolder {
  Q_OBJECT

  Q_PROPERTY(int index READ index WRITE set_index NOTIFY indexChanged)

signals:
  void indexChanged();

public:
  explicit ServerMethodHolder(QObject* p = nullptr) : MethodHolder(p) {}
  virtual ~ServerMethodHolder() {}
  int index() const { return index_; }
  void set_index(int v) {
    if (index_ != v) {
      deinit();
      index_ = v;
      indexChanged();
    }
  }
  void startProcessing() {
    Q_ASSERT(impl());
    impl()->startProcessing();
  }

  virtual ServerMethod* impl() const { return nullptr; }

  Q_INVOKABLE bool respond(int tag, const QVariant& data) {
    if (!impl()) {
      qWarning() << "Server method is not initialized.";
      return false;
    }
    return impl()->respond(tag, data);
  }

  Q_INVOKABLE bool abort(int tag, int error_code, const QString& message) {
    if (!impl()) {
      qWarning() << "Server method is not initialized.";
      return false;
    }
    return impl()->abort(tag, error_code, message);
  }

private:
  int index_ = -1;
};

class PROTOBUF_QML_DLLEXPORT ServerUnaryMethod : public ServerMethod {
  Q_OBJECT

public:
  explicit ServerUnaryMethod(QObject* p = nullptr) : ServerMethod(p) {}
  virtual ~ServerUnaryMethod() {}
};

class PROTOBUF_QML_DLLEXPORT ServerUnaryMethodHolder
    : public ServerMethodHolder {
  Q_OBJECT

public:
  explicit ServerUnaryMethodHolder(QObject* p = nullptr)
      : ServerMethodHolder(p) {}
  ~ServerUnaryMethodHolder() {}

  ServerMethod* impl() const final { return impl_.get(); }

  bool inject(ServerUnaryMethod* impl) {
    if (impl_) {
      qWarning() << "Server unary method is already initialized";
    }
    impl_.reset(impl);
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
    return true;
  }

protected:
  void deinit() final {
    // qWarning() << "Service method cannot be deinitialized.";
  }

private:
  std::unique_ptr<ServerUnaryMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT ServerReaderMethod : public ServerMethod {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ServerReaderMethod(QObject* p = nullptr) : ServerMethod(p) {}
  virtual ~ServerReaderMethod() {}
};

class PROTOBUF_QML_DLLEXPORT ServerReaderMethodHolder
    : public ServerMethodHolder {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ServerReaderMethodHolder(QObject* p = nullptr)
      : ServerMethodHolder(p) {}
  ~ServerReaderMethodHolder() {}

  ServerMethod* impl() const final { return impl_.get(); }

  bool inject(ServerReaderMethod* impl) {
    if (impl_) {
      qWarning() << "Server reader method is already initialized";
    }
    impl_.reset(impl);
    if (impl_) {
      connect(impl_.get(), &ServerReaderMethod::dataEnd, this,
              &ServerReaderMethodHolder::dataEnd);
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
    return true;
  }

protected:
  void deinit() final {}

private:
  std::unique_ptr<ServerReaderMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT ServerWriterMethod : public ServerMethod {
  Q_OBJECT

public:
  explicit ServerWriterMethod(QObject* p = nullptr) : ServerMethod(p) {}
  virtual ~ServerWriterMethod() {}
  virtual bool end(int tag) { return false; }
};

class PROTOBUF_QML_DLLEXPORT ServerWriterMethodHolder
    : public ServerMethodHolder {
  Q_OBJECT

public:
  explicit ServerWriterMethodHolder(QObject* p = nullptr)
      : ServerMethodHolder(p) {}
  ~ServerWriterMethodHolder() {}

  ServerMethod* impl() const final { return impl_.get(); }

  bool inject(ServerWriterMethod* impl) {
    if (impl_) {
      qWarning() << "Server writer method is already initialized";
    }
    impl_.reset(impl);
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
    return true;
  }

  Q_INVOKABLE bool end(int tag) {
    if (!impl_) {
      qWarning() << "Server writer method is not initialized.";
      return false;
    }
    return impl_->end(tag);
  }

protected:
  void deinit() final {
    // qWarning() << "Service method cannot be deinitialized.";
  }

private:
  std::unique_ptr<ServerWriterMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT ServerReaderWriterMethod : public ServerMethod {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ServerReaderWriterMethod(QObject* p = nullptr) : ServerMethod(p) {}
  virtual ~ServerReaderWriterMethod() {}
  virtual bool end(int tag) { return false; }
};

class PROTOBUF_QML_DLLEXPORT ServerReaderWriterMethodHolder
    : public ServerMethodHolder {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ServerReaderWriterMethodHolder(QObject* p = nullptr)
      : ServerMethodHolder(p) {}
  ~ServerReaderWriterMethodHolder() {}

  ServerMethod* impl() const final { return impl_.get(); }

  bool inject(ServerReaderWriterMethod* impl) {
    if (impl_) {
      qWarning() << "Server reader writer method is already initialized";
    }
    impl_.reset(impl);
    if (impl_) {
      connect(impl_.get(), &ServerReaderWriterMethod::dataEnd, this,
              &ServerReaderWriterMethodHolder::dataEnd);
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
    return true;
  }

  Q_INVOKABLE bool end(int tag) {
    if (!impl_) {
      qWarning() << "Server reader writer method is not initialized.";
      return false;
    }
    return impl_->end(tag);
  }

protected:
  void deinit() final {}

private:
  std::unique_ptr<ServerReaderWriterMethod> impl_;
};

class RpcServer;

class PROTOBUF_QML_DLLEXPORT RpcService : public QQuickItem {
  Q_OBJECT

  Q_PROPERTY(QVariantList methods READ variant_methods WRITE set_methods NOTIFY
                 methodsChanged);
  Q_PROPERTY(protobuf::qml::RpcServer* server READ server WRITE set_server
                 NOTIFY serverChanged);

signals:
  void methodsChanged();
  void serverChanged();

public:
  RpcService(QQuickItem* p = nullptr) : QQuickItem(p) {}

  QList<ServerMethodHolder*>& methods() { return methods_; }
  const QList<ServerMethodHolder*>& methods() const { return methods_; }
  const QVariantList& variant_methods() const { return var_methods_; }
  void set_methods(const QVariantList& v) {
    if (var_methods_ != v) {
      if (started()) {
        qWarning() << "Cannot assign methods once service is started.";
        return;
      }
      var_methods_ = v;
      methods_.clear();
      for (auto& v : var_methods_) {
        auto ptr = qobject_cast<ServerMethodHolder*>(v.value<QObject*>());
        if (ptr) {
          methods_.push_back(ptr);
        } else {
          qWarning() << "Unexpected type for rpc method.";
        }
      }
      methodsChanged();
    }
  }

  RpcServer* server() const { return server_; }
  void set_server(RpcServer* v);

  Q_INVOKABLE bool started() const;

private:
  QVariantList var_methods_;
  QList<ServerMethodHolder*> methods_;
  std::vector<QMetaObject::Connection> connections_;
  RpcServer* server_ = nullptr;
};

class PROTOBUF_QML_DLLEXPORT RpcServer : public QObject {
  Q_OBJECT

signals:
  void starting();
  void started();

public:
  explicit RpcServer(QObject* p = nullptr) : QObject(p) {}
  virtual ~RpcServer() {}
  virtual bool registerService(RpcService*) { return false; }
  bool has_started() const { return started_; }
  Q_INVOKABLE bool start() {
    if (started_) {
      qWarning() << "Server is already started";
      return false;
    }
    starting();
    started_ = doStart();
    if (started_) {
      started();
    }
    return started_;
  }

protected:
  virtual bool doStart() { return false; }

private:
  bool started_ = false;
};
}
}
#endif  // PROTOBUF_QML_SERVER_METHOD_H
