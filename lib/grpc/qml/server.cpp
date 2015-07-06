#include "grpc/qml/server.h"

namespace grpc {
namespace qml {

GrpcServer::~GrpcServer() {
  shutdown();
  if (thread_ && thread_->joinable()) {
    // TODO: Cancel jobs and wait for handling thread
    thread_->join();
  }
}

bool GrpcServer::registerService(::protobuf::qml::RpcService* service) {
  services_.emplace_back(service);
  auto& srv = services_.back();
  if (!srv.raw()) {
    services_.pop_back();
    qDebug() << "Failed to create service object.";
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
      if (auto unary =
              qobject_cast<::protobuf::qml::ServerUnaryMethodHolder*>(m)) {
        unary->inject(new ServerUnaryMethod(&srv, m->index(), cq_.get(),
                                            m->read_descriptor(),
                                            m->write_descriptor()));
      } else if (auto reader =
                     qobject_cast<::protobuf::qml::ServerReaderMethodHolder*>(
                         m)) {
        reader->inject(new ServerReaderMethod(&srv, m->index(), cq_.get(),
                                              m->read_descriptor(),
                                              m->write_descriptor()));
      } else if (auto writer =
                     qobject_cast<::protobuf::qml::ServerWriterMethodHolder*>(
                         m)) {
        writer->inject(new ServerWriterMethod(&srv, m->index(), cq_.get(),
                                              m->read_descriptor(),
                                              m->write_descriptor()));
      } else {
        qWarning() << "Currently, server streaming method is not supported.";
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
