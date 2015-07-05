#include "protobuf/qml/method.h"

namespace protobuf {
namespace qml {

bool UnaryMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerUnaryMethod(method_name(), read_descriptor(),
                                               write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
  } else {
    qWarning() << "Failed to initialize unary method implementation.";
  }
  return impl_ != nullptr;
}

bool WriterMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerWriterMethod(
        method_name(), read_descriptor(), write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
    }
  } else {
    qWarning() << "Failed to initialize writer method implementation.";
  }
  return impl_ != nullptr;
}

bool ReaderMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerReaderMethod(
        method_name(), read_descriptor(), write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &ReaderMethod::dataEnd, this,
              &ReaderMethodHolder::dataEnd);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
  } else {
    qWarning() << "Failed to initialize reader method implementation.";
    }
  }
  return impl_ != nullptr;
}

bool ReaderWriterMethodHolder::ensureInit() {
  if (!impl_ && readyForInit()) {
    impl_.reset(channel()->registerReaderWriterMethod(
        method_name(), read_descriptor(), write_descriptor()));
    if (impl_) {
      connect(impl_.get(), &MethodBase::data, this, &MethodHolder::data);
      connect(impl_.get(), &ReaderWriterMethod::dataEnd, this,
              &ReaderWriterMethodHolder::dataEnd);
      connect(impl_.get(), &MethodBase::error, this, &MethodHolder::error);
      connect(impl_.get(), &MethodBase::closed, this, &MethodHolder::closed);
  } else {
    qWarning() << "Failed to initialize bidi method implementation.";
    }
  }
  return impl_ != nullptr;
}
}
}
