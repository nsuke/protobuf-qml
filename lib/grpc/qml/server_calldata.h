#ifndef GRPC_QML_SERVER_CALLDATA_H
#define GRPC_QML_SERVER_CALLDATA_H

namespace grpc {
namespace qml {

class CallData {
public:
  virtual ~CallData() {}
  virtual void process(bool ok) = 0;

protected:
  CallData() {}

private:
  CallData(const CallData&) = delete;
  CallData& operator=(const CallData&) = delete;
};
}
}

#endif  // GRPC_QML_SERVER_CALLDATA_H
