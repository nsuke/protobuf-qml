#ifndef GRPC_QML_CLIENT_CALLDATA_H
#define GRPC_QML_CLIENT_CALLDATA_H

#include "grpc/qml/server_calldata.h"

#include <QDebug>

namespace grpc {
namespace qml {

template <typename Impl>
class ClientCallData : public CallData {
public:
  explicit ClientCallData(Impl* impl) : impl_(impl) {}
  virtual ~ClientCallData() {}
  grpc::Status& grpc_status() { return grpc_status_; }

  void reportGrpcError(std::string default_message) {
    auto ec = grpc::UNKNOWN;
    std::string msg(std::move(default_message));
    if (grpc_status().error_code()) {
      qWarning() << QString::fromStdString(grpc_status().error_message());
      ec = grpc_status().error_code();
      if (!grpc_status().error_message().empty()) {
        msg = grpc_status().error_message();
      }
    }
    impl_->method()->error(impl_->tag(), ec, msg.c_str());
  }

private:
  Impl* impl_;
  grpc::Status grpc_status_;
};
}
}
#endif  // GRPC_QML_CLIENT_CALLDATA_H
