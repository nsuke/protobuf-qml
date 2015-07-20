#include "protobuf/qml/server_method.h"

using namespace QV4;

namespace protobuf {
namespace qml {

void ServerMethodHolder::respond(QQmlV4Function* args) {
  if (!impl()) {
    qWarning() << "Server method is not initialized.";
    args->setReturnValue(Encode(false));
    return;
  }
  auto v4 = args->v4engine();
  Scope scope(v4);
  ScopedValue tag(scope, (*args)[0]);
  if (!tag || !tag->isNumber()) {
    qWarning() << "Invalid tag argument";
    args->setReturnValue(Encode(false));
    return;
  }
  ScopedArrayObject data(scope, (*args)[1]);
  if (!data) {
    qWarning() << "Invalid data argument";
    args->setReturnValue(Encode(false));
    return;
  }
  auto msg = write_descriptor()->v4()->jsValueToMessage(v4, *data);
  auto res = impl()->respond(tag->toInt32(), std::move(msg));
  args->setReturnValue(Encode(res));
}

void RpcService::set_server(RpcServer* v) {
  if (server_ != v) {
    if (started()) {
      qWarning() << "Cannot change server once started.";
      return;
    }
    if (v && v->has_started()) {
      qWarning() << "Cannot set running server to a service.";
      return;
    }
    if (server_) {
      for (auto& c : connections_) {
        disconnect(c);
      }
      connections_.clear();
    }
    if (v) {
      Q_ASSERT(connections_.empty());
      connections_.push_back(connect(v, &RpcServer::starting, this,
                                     [this, v] { v->registerService(this); }));
      connections_.push_back(connect(v, &RpcServer::started, this, [this, v] {
        for (auto m : methods_) {
          m->startProcessing();
        }
      }));
    }
    server_ = v;
    serverChanged();
  }
}

bool RpcService::started() const {
  return server_ && server_->has_started();
}
}
}
