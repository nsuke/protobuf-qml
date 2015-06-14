#include "grpc/qml/writer.h"
#include <iostream>

namespace grpc {
namespace qml {

class WriterWriteOp : public CallOp {
public:
  WriterWriteOp(WriterCall* call,
                std::unique_ptr<google::protobuf::Message> request)
      : call_(call), request_(std::move(request)) {
    std::cout << __PRETTY_FUNCTION__ << std::hex
              << reinterpret_cast<intptr_t>(this) << std::endl;
    ;
  }

  ~WriterWriteOp() { qDebug() << __PRETTY_FUNCTION__; }

  virtual void onEvent(bool* handled) { qDebug() << __PRETTY_FUNCTION__; }
  virtual void onError(bool) {
    qDebug() << __PRETTY_FUNCTION__;
    // TODO: Is is correct to end the call on write failure ?
    call_->method()->error(call_->tag(), "Failed to write to channel");
    call_->finish();
  }

  google::protobuf::Message* request() { return request_.get(); }

private:
  WriterCall* call_;
  std::unique_ptr<google::protobuf::Message> request_;
};

class WriterFinishOp : public CallOp {
public:
  WriterFinishOp(WriterCall* call) : call_(call) {
    std::cout << __PRETTY_FUNCTION__ << std::hex
              << reinterpret_cast<intptr_t>(this) << std::endl;
    ;
  }

  ~WriterFinishOp() {
    qDebug() << __PRETTY_FUNCTION__;
    call_->method()->deleteCall(call_->tag());
  }

  virtual void onEvent(bool* handled) {
    qDebug() << __PRETTY_FUNCTION__;
    call_->method()->data(
        call_->tag(),
        call_->read_descriptor()->dataFromMessage(*call_->response()));
  }

  virtual void onError(bool) {
    qDebug() << __PRETTY_FUNCTION__;
    // TODO: Is is correct to end the call on write failure ?
    call_->method()->error(call_->tag(), "Error while finishing.");
  }

private:
  WriterCall* call_;
};

class SimpleHandler : public CallOp {
public:
  // template <typename F, typename E, typename D>
  // SimpleOp(F f, E e, D d)
  //     : f_(new std::function<void(bool*)>(f)),
  //       err_(new std::function<void(bool)>(e)),
  //       del_(new std::function<void()>(d))

  // {}

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

std::unique_ptr<SimpleHandler> emptyHandler() {
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

void WriterCall::finish() {
  qDebug() << __PRETTY_FUNCTION__;
  if (writer_) {
    std::unique_ptr<WriterFinishOp> op(new WriterFinishOp(this));
    auto status = &op->status;
    writer_->Finish(status, op.release());
  }
}

void WriterCall::ensureInit() {
  if (!writer_) {
    response_.reset(read_desc_->newMessage());
    writer_.reset(new grpc::ClientAsyncWriter<google::protobuf::Message>(
        channel_, cq_, method_->raw(), &context_, response_.get(),
        emptyHandler().release()));
  }
}

bool WriterCall::write(std::unique_ptr<google::protobuf::Message> request) {
  ensureInit();
  std::unique_ptr<WriterWriteOp> op(
      new WriterWriteOp(this, std::move(request)));
  auto req = op->request();
  writer_->Write(*req, op.release());
  return true;
}

bool WriterCall::writesDone(int timeout) {
  qDebug() << __PRETTY_FUNCTION__;
  ensureInit();
  if (timeout >= 0) {
    context_.set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(timeout));
  } else {
    // TODO: set to infinite
  }
  auto handler = deleteHandler([this] { finish(); });
  writer_->WritesDone(handler.release());
  return true;
}

bool WriterMethod::write(int tag, const QVariant& data, int timeout) {
  // TODO: handle timeout
  std::unique_ptr<google::protobuf::Message> request(
      write_->dataToMessage(data));
  if (!request) {
    error(tag, "Failed to convert to message object.");
    return false;
  }
  WriterCall* call = nullptr;
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    auto it = calls_.find(tag);
    if (it == calls_.end()) {
      auto res = calls_.emplace(
          std::piecewise_construct, std::forward_as_tuple(tag),
          std::forward_as_tuple(tag, channel_.get(), cq_, this, read_));
      if (!res.second) {
        error(tag, "Failed to create call object");
        return false;
      }
      call = &res.first->second;
    } else {
      call = &it->second;
    }
  }
  Q_ASSERT(call);
  return call->write(std::move(request));
}

bool WriterMethod::writesDone(int tag, int timeout) {
  WriterCall* call = nullptr;
  {
    std::lock_guard<std::mutex> lock(calls_mutex_);
    auto it = calls_.find(tag);
    if (it == calls_.end()) {
      // not found
      qWarning() << "Tag not found " << tag;
      return false;
    }
    call = &it->second;
  }
  Q_ASSERT(call);
  return call->writesDone(timeout);
}
}
}
