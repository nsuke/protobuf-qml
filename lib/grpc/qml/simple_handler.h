#ifndef GRPC_QML_SIMPLE_HANDLER_H
#define GRPC_QML_SIMPLE_HANDLER_H

#include "grpc/qml/base.h"
#include <QtGlobal>
#include <QDebug>
#include <memory>

namespace grpc {
namespace qml {

class SimpleHandler : public CallOp {
public:
  template <typename F>
  void setDeleteHandler(F f) {
    qDebug() << __PRETTY_FUNCTION__;
    del_.reset(new std::function<void()>(f));
  }

  template <typename F>
  void setErrorHandler(F f) {
    qDebug() << __PRETTY_FUNCTION__;
    err_.reset(new std::function<void(bool)>(f));
  }

  template <typename F>
  void setEventHandler(F f) {
    qDebug() << __PRETTY_FUNCTION__;
    f_.reset(new std::function<void(bool*)>(f));
  }

  void onEvent(bool* handled) {
    qDebug() << __PRETTY_FUNCTION__;
    if (f_) {
      (*f_)(handled);
    }
  }

  void onError(bool timeout) {
    qDebug() << __PRETTY_FUNCTION__;
    if (err_) {
      (*err_)(timeout);
    }
  }

  ~SimpleHandler() {
    qDebug() << __PRETTY_FUNCTION__;
    if (del_) {
      (*del_)();
    }
  }

private:
  std::unique_ptr<std::function<void(bool*)>> f_;
  std::unique_ptr<std::function<void(bool)>> err_;
  std::unique_ptr<std::function<void()>> del_;
};

inline std::unique_ptr<SimpleHandler> emptyHandler() {
  return std::unique_ptr<SimpleHandler>(new SimpleHandler);
}

template <typename F>
std::unique_ptr<SimpleHandler> errorHandler(F f) {
  std::unique_ptr<SimpleHandler> handler(new SimpleHandler);
  handler->setErrorHandler(f);
  return std::move(handler);
}

template <typename F>
std::unique_ptr<SimpleHandler> deleteHandler(F f) {
  qDebug() << __PRETTY_FUNCTION__;
  std::unique_ptr<SimpleHandler> handler(new SimpleHandler);
  handler->setDeleteHandler(f);
  return std::move(handler);
}
}
}
#endif  // GRPC_QML_SIMPLE_HANDLER_H
