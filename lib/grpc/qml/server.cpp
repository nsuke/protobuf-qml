#include "grpc/qml/server.h"
#include "grpc/qml/server_bidi.h"
#include "grpc/qml/server_reader.h"
#include "grpc/qml/server_unary.h"
#include "grpc/qml/server_writer.h"

namespace grpc {
namespace qml {

GrpcServer::~GrpcServer() {
  shutdown();
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
    qWarning() << "Need address and credentials to start";
    return false;
  }
  ::grpc::ServerBuilder build;
  build.AddListeningPort(address_.toStdString(), credentials_->raw());
  auto cq = build.AddCompletionQueue();
  for (auto& srv : services_) {
    for (auto m : srv.service()->methods()) {
      auto type = ::grpc::RpcMethod::NORMAL_RPC;
      if (auto unary =
              qobject_cast<::protobuf::qml::ServerUnaryMethodHolder*>(m)) {
        unary->inject(new ServerUnaryMethod(&srv, m->index(), cq.get(),
                                            m->read_descriptor(),
                                            m->write_descriptor()));
      } else if (auto reader =
                     qobject_cast<::protobuf::qml::ServerReaderMethodHolder*>(
                         m)) {
        reader->inject(new ServerReaderMethod(&srv, m->index(), cq.get(),
                                              m->read_descriptor(),
                                              m->write_descriptor()));
        type = ::grpc::RpcMethod::CLIENT_STREAMING;
      } else if (auto writer =
                     qobject_cast<::protobuf::qml::ServerWriterMethodHolder*>(
                         m)) {
        writer->inject(new ServerWriterMethod(&srv, m->index(), cq.get(),
                                              m->read_descriptor(),
                                              m->write_descriptor()));
        type = ::grpc::RpcMethod::SERVER_STREAMING;
      } else if (auto bidi = qobject_cast<
                     ::protobuf::qml::ServerReaderWriterMethodHolder*>(m)) {
        bidi->inject(new ServerBidiMethod(&srv, m->index(), cq.get(),
                                          m->read_descriptor(),
                                          m->write_descriptor()));
        type = ::grpc::RpcMethod::BIDI_STREAMING;
      } else {
        qWarning() << "Failed to register service method: Unknown method type.";
      }
      srv.addMethod(m->method_name(), type);
    }
    build.RegisterService(srv.raw());
  }
  server_ = build.BuildAndStart();
  if (!server_) {
    qWarning() << "Failed to initialize gRPC server";
    services_.clear();
    return false;
  } else {
    cq_.swap(cq);
    Q_ASSERT(!thread_);
    thread_.reset(new std::thread([this] { handle(); }));
    return true;
  }
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
