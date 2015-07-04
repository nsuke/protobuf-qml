#ifndef GRPC_QML_H
#define GRPC_QML_H

#include "grpc/qml/credentials.h"
#include "protobuf/qml/method.h"
#include <grpc++/config.h>
#include <grpc++/stream.h>
#include <grpc++/channel_arguments.h>
#include <grpc++/create_channel.h>
#include <grpc++/impl/rpc_method.h>
#include <QObject>
#include <thread>

namespace grpc {
namespace qml {

class Channel : public ::protobuf::qml::Channel2 {
  Q_OBJECT

  Q_PROPERTY(QString target READ target WRITE set_target NOTIFY targetChanged)
  Q_PROPERTY(grpc::qml::Credentials* credentials READ credentials WRITE set_credentials
                 NOTIFY credentialsChanged)

signals:
  void engineChanged();
  void credentialsChanged();
  void targetChanged();

public:
  Channel(QObject* p = nullptr);
  ~Channel();
  ::protobuf::qml::UnaryMethod* registerUnaryMethod(
      const QString& name,
      ::protobuf::qml::DescriptorWrapper* read,
      ::protobuf::qml::DescriptorWrapper* write) final;

  ::protobuf::qml::ReaderMethod* registerReaderMethod(
      const QString& name,
      ::protobuf::qml::DescriptorWrapper* read,
      ::protobuf::qml::DescriptorWrapper* write) final;

  ::protobuf::qml::WriterMethod* registerWriterMethod(
      const QString& name,
      ::protobuf::qml::DescriptorWrapper* read,
      ::protobuf::qml::DescriptorWrapper* write) final;

  ::protobuf::qml::ReaderWriterMethod* registerReaderWriterMethod(
      const QString& name,
      ::protobuf::qml::DescriptorWrapper* read,
      ::protobuf::qml::DescriptorWrapper* write) final;

  const QString& target() const { return target_; }
  void set_target(const QString& target) {
    if (target != target_) {
      target_ = target;
      targetChanged();
    }
  }

  Credentials* credentials() const { return creds_; }
  void set_credentials(Credentials* creds) {
    if (creds != creds_) {
      creds_ = creds;
      credentialsChanged();
    }
  }

private:
  bool ensureInit();
  void discardMethods();
  void startThread();
  void shutdown();

  std::unique_ptr<grpc::CompletionQueue> cq_;
  std::shared_ptr<grpc::ChannelInterface> raw_;

  QString target_;
  Credentials* creds_ = nullptr;

  std::unique_ptr<std::thread> thread_;
};
}
}

#endif  // GRPC_QML_H
