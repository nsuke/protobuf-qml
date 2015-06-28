#include "protobuf/qml/server_method.h"

namespace protobuf {
namespace qml {

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
