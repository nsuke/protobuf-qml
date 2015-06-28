#include "grpc/qml/server.h"
namespace grpc {
namespace qml {

ServerUnaryMethod::ServerUnaryMethod(GrpcService* service,
                                     int index,
                                     ::grpc::ServerCompletionQueue* cq,
                                     ::protobuf::qml::DescriptorWrapper* read,
                                     ::protobuf::qml::DescriptorWrapper* write)
    : read_(read),
      write_(write),
      cq_(cq),
      index_(index),
      service_(service) {
}

void ServerUnaryMethod::startProcessing() {
  new ServerUnaryCallData(this, service_, index_, cq_, read_, write_);
}

void ServerUnaryMethod::onRequest(ServerUnaryCallData* cdata) {
  qDebug() << " ON REQ";
  auto tag = store(cdata);
  data(tag, cdata->data());
}

bool ServerUnaryMethod::respond(int tag, const QVariant& data) {
  auto cdata = remove(tag);
  if (!cdata) {
    qWarning() << "";
    return false;
  }
  cdata->resume(data);
  return true;
}

ServerUnaryCallData::ServerUnaryCallData(
      ServerUnaryMethod* method,
    GrpcService* service,
    int index,
    ::grpc::ServerCompletionQueue* cq,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write)
    : read_(read),
      write_(write),
      method_(method),
      writer_(&context_),
      cq_(cq),
      index_(index),
      service_(service) {
  process(true);
}

void ServerUnaryCallData::process(bool ok) {
  if (status_ == Status::INIT) {
    request_.reset(read_->newMessage());
    qDebug() << "REQUESTING";
    service_->raw()->RequestAsyncUnary(index_, &context_, request_.get(),
                                       &writer_, cq_, cq_, this);
    status_ = Status::READ;
  } else if (status_ == Status::READ) {
    qDebug() << "READ";
    if (!ok) {
      // TODO:
      qWarning() << "Status not OK.";
    }
    data_ = read_->dataFromMessage(*request_);
    status_ = Status::FROZEN;
    method_->onRequest(this);
  } else if (status_ == Status::WRITE) {
    qDebug() << "WRITE";
    if (!ok) {
      // notify
    }
    status_ = Status::DONE;
    writer_.Finish(grpc::Status::OK, this);
  } else if (status_ == Status::DONE) {
    new ServerUnaryCallData(method_, service_, index_, cq_, read_, write_);
    if (!ok) {
      // notify
    }
    delete this;
  } else {
    Q_ASSERT(false);
  }
}

void ServerUnaryCallData::resume(const QVariant& data) {
  if (status_ != Status::FROZEN) {
    qWarning() << "Resume called for non-frozen call data.";
    return;
  }
  response_.reset(write_->dataToMessage(data));
  if (!response_) {
    // TODO: how to abort from here ?
  }
  status_ = Status::WRITE;
  writer_.Write(*response_, this);
}

GrpcServer::~GrpcServer() {
  shutdown();
  if (thread_ && thread_->joinable()) {
    thread_->detach();
  }
}

bool GrpcServer::registerService(::protobuf::qml::RpcService* service) {
  services_.emplace_back(service);
  auto& srv = services_.back();
  if (!srv.raw()) {
    services_.pop_back();
    qDebug() << "FAIL";
    return false;
  }
  return true;
}

bool GrpcServer::doStart() {
  if (address_.isEmpty() || !credentials_ || !credentials_->raw()) {
    return false;
  }
  ::grpc::ServerBuilder build;
  build.AddListeningPort(address_.toStdString(), credentials_->raw());
  cq_ = build.AddCompletionQueue();
  for (auto& srv : services_) {
    for (auto m : srv.service()->methods()) {
      auto unary = qobject_cast<::protobuf::qml::ServerUnaryMethodHolder*>(m);
      if (unary) {
        unary->inject(new ServerUnaryMethod(&srv, m->index(), cq_.get(),
                                            m->read_descriptor(),
                                            m->write_descriptor()));
      } else {
        qWarning() << "Currently, only unary method is supported.";
      }
    }
    build.RegisterAsyncService(srv.raw());
  }
  server_ = build.BuildAndStart();
  if (!server_) {
    qWarning() << "Failed to initialize gRPC server";
    services_.clear();
  } else {
    Q_ASSERT(!thread_);
    thread_.reset(new std::thread([this] { handle(); }));
  }
  return server_ != nullptr;
}

void GrpcServer::handle() {
  void* tag = nullptr;
  bool ok = false;
  while (true) {
    if (!cq_->Next(&tag, &ok)) {
      cq_.reset();
      return;
    }
    auto cdata = static_cast<CallData*>(tag);
    tag = nullptr;
    if (cdata) {
      cdata->process(ok);
    }
  }
}
}
}
