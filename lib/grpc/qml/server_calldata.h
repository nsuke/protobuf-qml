#ifndef GRPC_QML_SERVER_CALLDATA_H
#define GRPC_QML_SERVER_CALLDATA_H

#include <unordered_map>
#include <mutex>

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

template <typename CallData>
class CallDataStore {
protected:
  int store(CallData* cdata) {
    std::lock_guard<std::mutex> lock(cdata_mutex_);
    return doStore(cdata);
  }

  int doStore(CallData* cdata) {
    auto res = cdata_.insert(std::make_pair(++tag_, cdata));
    if (res.second) {
      return res.first->first;
    } else {
      return doStore(cdata);
    }
  }

  CallData* remove(int tag) {
    std::lock_guard<std::mutex> lock(cdata_mutex_);
    auto it = cdata_.find(tag);
    if (it == cdata_.end()) {
      return nullptr;
    }
    auto ptr = it->second;
    cdata_.erase(it);
    return ptr;
  }

  int tag_ = 1;

private:
  std::mutex cdata_mutex_;
  std::unordered_map<int, CallData*> cdata_;
};
}
}

#endif  // GRPC_QML_SERVER_CALLDATA_H
