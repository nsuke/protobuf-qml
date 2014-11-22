#ifndef QPB_IO_H
#define QPB_IO_H

#include "qpb/common.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <QObject>

namespace qpb {

template <typename T, typename Stream>
class ScopedSession {
 public:
  ScopedSession() : ScopedSession(nullptr, nullptr) {}

  ScopedSession(T* t, Stream* s) : t_(t), s_(s) {}

  ScopedSession(ScopedSession&& other) { *this = std::move(other); }

  ScopedSession& operator=(ScopedSession&& other) {
    t_ = other.t_;
    s_ = other.s_;
    other.t_ = nullptr;
    other.s_ = nullptr;
    return *this;
  }

  ~ScopedSession() { reset(); }

  void reset() {
    if (s_) {
      delete s_;
      s_ = nullptr;
      if (t_) {
        t_->notify();
        t_ = nullptr;
      }
    }
  }

  Stream* stream() const { return s_; }

  operator bool() { return static_cast<bool>(s_); }

 private:
  ScopedSession(const ScopedSession&) = delete;
  ScopedSession& operator=(const ScopedSession&) = delete;

  T* t_;
  Stream* s_;
};

class QPB_DLLEXPORT InputDevice : public QObject {
  Q_OBJECT

signals:
  void dataReceived();

 public:
  typedef ScopedSession<InputDevice, google::protobuf::io::ZeroCopyInputStream>
      SessionPtr;

  InputDevice(QObject* p = nullptr) : QObject(p) {}

  virtual ~InputDevice() {}

  virtual SessionPtr createSession();

  virtual void notify() {}
};

class QPB_DLLEXPORT OutputDevice : public QObject {
  Q_OBJECT

signals:
  void dataWritten();

 public:
  typedef ScopedSession<OutputDevice,
                        google::protobuf::io::ZeroCopyOutputStream> SessionPtr;

  OutputDevice(QObject* p = nullptr) : QObject(p) {}

  virtual ~OutputDevice() {}

  virtual SessionPtr createSession();

  virtual void notify() {}
};
}

#endif  // QPB_IO_H
