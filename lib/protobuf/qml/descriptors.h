#ifndef PROTOBUF_QML_DESCRIPTOR_DATABASE_H
#define PROTOBUF_QML_DESCRIPTOR_DATABASE_H

#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <QObject>
#include <QByteArray>
#include <QVariant>
#include <QThread>
#include <QThreadStorage>
#include <memory>

namespace protobuf {
namespace qml {

class DescriptorWrapper;

class DescriptorWrapper : public QObject {
  Q_OBJECT

signals:
  void parseCompleted(int key, QVariant result, bool error);
  void serializeCompleted(int key, bool error);
  void maxThreadsChanged();

public:
  DescriptorWrapper(const google::protobuf::Descriptor* descriptor,
                    QObject* p = nullptr)
      : QObject(p), descriptor_(descriptor) {}
  ~DescriptorWrapper();

  void clearSharedMessage() { message_.setLocalData(nullptr); }

  google::protobuf::Message* dataToMessage(const QVariant& msgData);

  QVariant dataFromMessage(const google::protobuf::Message& msg);

  google::protobuf::Message* newMessage() {
    return message_factory_.GetPrototype(descriptor_)->New();
  }

private:
  google::protobuf::Message* sharedMessage() {
    if (!message_.hasLocalData()) message_.setLocalData(newMessage());
    return message_.localData();
  }

  int key_ = 0;
  const google::protobuf::Descriptor* descriptor_;
  google::protobuf::DynamicMessageFactory message_factory_;
  QThreadStorage<google::protobuf::Message*> message_;
};

class FileDescriptorWrapper : public QObject {
  Q_OBJECT

public:
  FileDescriptorWrapper(const google::protobuf::FileDescriptor* descriptor,
                        QObject* p = nullptr)
      : QObject(p), descriptor_(descriptor) {}

  Q_INVOKABLE protobuf::qml::DescriptorWrapper* messageType(int i) {
    if (descriptor_) {
      if (auto p = descriptor_->message_type(i)) {
        return new DescriptorWrapper(p, this);
      }
    }
    return nullptr;
  }

private:
  const google::protobuf::FileDescriptor* descriptor_;
};

class DescriptorPoolWrapper : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE protobuf::qml::FileDescriptorWrapper* addFileDescriptor(QVariant);

private:
  google::protobuf::DescriptorPool pool_;
  // We keep track of "children" object to control object delete order.
  // Since Qt deleteChildren occurs *after* deleting fileds, we cannot delete
  // descriptors *before* deleting DescriptorPool using parent-child tree
  // mechanism.
  std::vector<std::unique_ptr<FileDescriptorWrapper>> children_;
};
}
}
#endif  // PROTOBUF_QML_DESCRIPTOR_DATABASE_H
