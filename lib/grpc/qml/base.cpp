#include "grpc/qml/server_calldata.h"
#include "grpc/qml/base.h"
#include "grpc/qml/unary.h"
#include "grpc/qml/writer.h"
#include "grpc/qml/reader.h"

namespace grpc {
namespace qml {

Channel::Channel(QObject* p) : ::protobuf::qml::Channel2(p) {
}

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
  if (thread_ && thread_->joinable()) {
    thread_->join();
  }
  thread_.reset();
}

void Channel::startThread() {
  cq_.reset(new grpc::CompletionQueue);
  thread_.reset(new std::thread([this] {
    void* tag = nullptr;
    bool ok = false;
    bool handled = false;
    for (;;) {
      if (!cq_->Next(&tag, &ok)) {
        qDebug() << "Shutting down";
        cq_.reset();
        return;
      }
      auto cdata = static_cast<CallData*>(tag);
      tag = nullptr;
      if (cdata) {
        cdata->process(ok);
      }
    }
  }));
}

bool Channel::ensureInit() {
  if (!raw_) {
    qDebug() << "Initialize with creds "
             << (creds_ ? creds_->raw().get() : nullptr);
    grpc::ChannelArguments null_args;
    auto before = raw_.get();
    raw_ = grpc::CreateChannel(target_.toStdString(),
                               creds_ ? creds_->raw() : nullptr, null_args);
    if (before != raw_.get()) {
      discardMethods();
    }
  }

  if (raw_ && !thread_) {
    if (raw_) {
      startThread();
    }
  }

  return raw_ != nullptr;
}

::protobuf::qml::UnaryMethod* Channel::registerUnaryMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  if (!ensureInit()) {
    return nullptr;
  }
  Q_ASSERT(cq_);
  return new UnaryMethod(name.toStdString(), read, write, raw_, cq_.get());
}
::protobuf::qml::ReaderMethod* Channel::registerReaderMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  return !ensureInit() ? nullptr : new ReaderMethod(name.toStdString(), read,
                                                    write, raw_, cq_.get());
  return nullptr;
}
::protobuf::qml::WriterMethod* Channel::registerWriterMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  return !ensureInit() ? nullptr : new WriterMethod(name.toStdString(), read,
                                                    write, raw_, cq_.get());
  return nullptr;
}
::protobuf::qml::ReaderWriterMethod* Channel::registerReaderWriterMethod(
    const QString& name,
    ::protobuf::qml::DescriptorWrapper* read,
    ::protobuf::qml::DescriptorWrapper* write) {
  // TODO: Not implemented yet
  // return !ensureInit() ? nullptr
  //                      : new ReaderWriterMethod(name.toStdString(), read, write,
  //                                               raw_, cq_.get());
  return nullptr;
}
}
}
