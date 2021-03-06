#include "grpc/qml/base.h"
#include "grpc/qml/reader.h"
#include "grpc/qml/reader_writer.h"
#include "grpc/qml/server_calldata.h"
#include "grpc/qml/unary.h"
#include "grpc/qml/writer.h"

#include <grpc++/completion_queue.h>

namespace grpc {
namespace qml {

Channel::Channel(QObject* p) : ::protobuf::qml::Channel2(p) {}

Channel::~Channel() {
  shutdown();
}

void Channel::discardMethods() {
  // TODO: Force methods to reinitialize
}

void Channel::shutdown() {
  if (cq_) {
    cq_->Shutdown();
  }
  if (work_.valid()) {
    work_.wait();
  }
}

void Channel::startThread() {
  cq_.reset(new grpc::CompletionQueue);

  work_ = std::async(std::launch::async, [this] {
    void* tag = nullptr;
    bool ok = false;
    for (;;) {
      if (!cq_->Next(&tag, &ok)) {
        qInfo() << "completed processing";
        cq_.reset();
        return;
      }
      auto cdata = static_cast<CallData*>(tag);
      tag = nullptr;
      if (cdata) {
        cdata->process(ok);
      } else {
        qWarning() << "no calldata";
      }
    }
  });
}

bool Channel::ensureInit() {
  if (!raw_) {
    grpc::ChannelArguments null_args;
    auto before = raw_.get();
    raw_ = grpc::CreateCustomChannel(
        target_.toStdString(), creds_ ? creds_->raw() : nullptr, null_args);
    if (before != raw_.get()) {
      discardMethods();
    }
  }

  if (raw_ && !work_.valid()) {
    startThread();
    Q_ASSERT(cq_);
  }

  return raw_ != nullptr;
}

::protobuf::qml::UnaryMethod* Channel::registerUnaryMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  return !ensureInit() ? nullptr : new UnaryMethod(name.toStdString(), read,
                                                   write, raw_, cq_.get());
}
::protobuf::qml::ReaderMethod* Channel::registerReaderMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  return !ensureInit() ? nullptr : new ReaderMethod(name.toStdString(), read,
                                                    write, raw_, cq_.get());
}
::protobuf::qml::WriterMethod* Channel::registerWriterMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  return !ensureInit() ? nullptr : new WriterMethod(name.toStdString(), read,
                                                    write, raw_, cq_.get());
}
::protobuf::qml::ReaderWriterMethod* Channel::registerReaderWriterMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  return !ensureInit() ? nullptr
                       : new ReaderWriterMethod(name.toStdString(), read, write,
                                                raw_, cq_.get());
}
}
}
