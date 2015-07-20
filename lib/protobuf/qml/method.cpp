#include "protobuf/qml/method.h"

using namespace QV4;

namespace protobuf {
namespace qml {

template <typename Holder, typename Impl>
void v4write_with_timeout(Holder* that, Impl* impl, QQmlV4Function* args) {
  auto v4 = args->v4engine();
  Scope scope(v4);
  ScopedValue arg_tag(scope, (*args)[0]);
  if (!arg_tag || !arg_tag->isNumber()) {
    // TODO: throw error
    qWarning() << "tag argument is invalid";
    args->setReturnValue(Encode(false));
    return;
  }
  auto tag = arg_tag->toInt32();
  ScopedArrayObject arg_data(scope, (*args)[1]);
  if (!arg_data) {
    qWarning() << "data argument is invalid";
    args->setReturnValue(Encode(false));
    return;
  }
  auto msg = that->write_descriptor()->v4()->jsValueToMessage(v4, *arg_data);
  if (!msg) {
    qWarning() << "Failed to create message object";
    args->setReturnValue(Encode(false));
    return;
  }
  ScopedValue arg_timeout(scope, (*args)[2]);
  int timeout = -1;
  if (arg_timeout && arg_timeout->isInteger()) {
    timeout = arg_timeout->toInt32();
  }

  auto result = impl->write(tag, std::move(msg), timeout);
  args->setReturnValue(Encode(result));
}

template <typename Holder, typename Impl>
void v4write(Holder* that, Impl* impl, QQmlV4Function* args) {
  auto v4 = args->v4engine();
  Scope scope(v4);
  ScopedValue arg_tag(scope, (*args)[0]);
  if (!arg_tag || !arg_tag->isNumber()) {
    // TODO: throw error
    qWarning() << "tag argument is invalid";
    args->setReturnValue(Encode(false));
    return;
  }
  auto tag = arg_tag->toInt32();
  ScopedArrayObject arg_data(scope, (*args)[1]);
  if (!arg_data) {
    qWarning() << "data argument is invalid";
    args->setReturnValue(Encode(false));
    return;
  }
  auto msg = that->write_descriptor()->v4()->jsValueToMessage(v4, *arg_data);
  if (!msg) {
    qWarning() << "Failed to create message object";
    args->setReturnValue(Encode(false));
    return;
  }

  auto result = impl->write(tag, std::move(msg));
  args->setReturnValue(Encode(result));
}

void MethodHolder::set_read_descriptor(DescriptorWrapper* read_desc) {
  if (read_desc != read_desc_) {
    deinit();
    read_desc_ = read_desc;
    readDescriptorChanged();
  }
}

void MethodHolder::set_write_descriptor(DescriptorWrapper* write_desc) {
  if (write_desc != write_desc_) {
    deinit();
    write_desc_ = write_desc;
    writeDescriptorChanged();
  }
}

void MethodHolder::handleData(
    int tag, const std::shared_ptr<google::protobuf::Message>& msg) {
  if (!v4) {
    v4 = QV8Engine::getV4(qjsEngine(this));
  }
  if (!v4 || !read_desc_) {
    qWarning() << "V4 engine and/or descriptor not available";
    return;
  }
  if (!msg) {
    data(tag, QJSValue(v4, QV4::Encode::null()));
    return;
  }
  Scope scope(v4);
  ScopedArrayObject js_data(scope,
                            read_desc_->v4()->messageToJsValue(v4, *msg));
  if (!js_data) {
    qWarning() << "No data to handle";
    return;
  }
  data(tag, QJSValue(v4, js_data->asReturnedValue()));
}

void UnaryMethodHolder::write(QQmlV4Function* args) {
  if (!ensureInit()) {
    args->setReturnValue(Encode(false));
    return;
  }
  v4write_with_timeout(this, impl_.get(), args);
}

void WriterMethodHolder::write(QQmlV4Function* args) {
  if (!ensureInit()) {
    args->setReturnValue(QV4::Encode(false));
    return;
  }
  v4write(this, impl_.get(), args);
}

void ReaderMethodHolder::write(QQmlV4Function* args) {
  if (!ensureInit()) {
    args->setReturnValue(QV4::Encode(false));
    return;
  }
  v4write_with_timeout(this, impl_.get(), args);
}

void ReaderWriterMethodHolder::write(QQmlV4Function* args) {
  if (!ensureInit()) {
    args->setReturnValue(QV4::Encode(false));
    return;
  }
  v4write(this, impl_.get(), args);
}

bool UnaryMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerUnaryMethod(method_name(), read_descriptor(),
                                               write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::handleData);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
  } else {
    // qWarning() << "Failed to initialize unary method implementation.";
  }
  return impl_ != nullptr;
}

bool WriterMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerWriterMethod(
        method_name(), read_descriptor(), write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::handleData);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
  } else {
    // qWarning() << "Failed to initialize writer method implementation.";
  }
  return impl_ != nullptr;
}

bool ReaderMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerReaderMethod(
        method_name(), read_descriptor(), write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::handleData);
      connect(impl_.get(), &ReaderMethod::dataEnd, this,
              &ReaderMethodHolder::dataEnd);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    } else {
      // qWarning() << "Failed to initialize reader method implementation.";
    }
  }
  return impl_ != nullptr;
}

bool ReaderWriterMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerReaderWriterMethod(
        method_name(), read_descriptor(), write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::handleData);
      connect(impl_.get(), &ReaderWriterMethod::dataEnd, this,
              &ReaderWriterMethodHolder::dataEnd);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    } else {
      // qWarning() << "Failed to initialize bidi method implementation.";
    }
  }
  return impl_ != nullptr;
}
}
}
