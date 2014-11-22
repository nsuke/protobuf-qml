#include "qpb/io.h"
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

namespace qpb {

class DescriptorWrapper;

class AsyncProcessor : public QThread {
  Q_OBJECT

signals:
  void startParse(int key, InputDevice* input);
  void startSerialize(int key, OutputDevice* output, QVariantMap value);

 public:
  AsyncProcessor(DescriptorWrapper* parent)
      : QThread(reinterpret_cast<QObject*>(parent)), parent_(parent) {
    moveToThread(this);
    connect(this,
            SIGNAL(startParse(int, InputDevice*)),
            this,
            SLOT(doParse(int, InputDevice*)));
    connect(this,
            SIGNAL(startSerialize(int, OutputDevice*, const QVariantMap&)),
            this,
            SLOT(doSerialize(int, OutputDevice*, QVariantMap)));
  }

  bool has_task() const { return has_task_; }

 private slots:
  void doParse(int key, InputDevice* input);
  void doSerialize(int key, OutputDevice* input, QVariantMap value);

 private:
  bool has_task_ = false;
  DescriptorWrapper* parent_;
};

class DescriptorWrapper : public QObject {
  Q_OBJECT
  Q_PROPERTY(int maxThreads READ max_threads WRITE set_max_threads NOTIFY
                 maxThreadsChanged)

signals:
  void parseCompleted(int key, QVariant result);
  void serializeCompleted(int key);
  void maxThreadsChanged();

 public:
  DescriptorWrapper(const google::protobuf::Descriptor* descriptor,
                    QObject* p = 0)
      : QObject(p), descriptor_(descriptor) {}
  ~DescriptorWrapper();

  Q_INVOKABLE QVariant parse(InputDevice* input);
  Q_INVOKABLE bool serialize(OutputDevice* output, QVariantMap value);

  Q_INVOKABLE int parseAsync(InputDevice* input) {
    auto i = async();
    if (!i) return 0;
    async_[i]->startParse(++key_, input);
    return key_;
  }
  Q_INVOKABLE int serializeAsync(OutputDevice* output, QVariantMap value) {
    auto i = async();
    if (!i) return 0;
    async_[i]->startSerialize(++key_, output, std::move(value));
    return key_;
  }

  int max_threads() const { return max_threads_; }
  void set_max_threads(int value) {
    if (max_threads_ != value) {
      max_threads_ = value;
      maxThreadsChanged();
    };
  }

 private:
  int async() {
    size_t i = 0;
    for (; i < async_.size(); i++) {
      if (!async_[i]->has_task()) break;
    }
    if (i == async_.size()) {
      if (i <= static_cast<size_t>(max_threads_)) {
        async_.emplace_back(new AsyncProcessor(this));
      } else {
        return 0;
      }
    }
    return i;
  }

  google::protobuf::Message* newMessage() {
    return message_factory_.GetPrototype(descriptor_)->New();
  }

  google::protobuf::Message* sharedMessage() {
    if (!message_.hasLocalData()) message_.setLocalData(newMessage());
    return message_.localData();
  }

  int max_threads_ = 8;
  int key_ = 0;
  const google::protobuf::Descriptor* descriptor_;
  google::protobuf::DynamicMessageFactory message_factory_;
  QThreadStorage<google::protobuf::Message*> message_;
  // Use pointer because QObject does not have move constructor as of writing.
  std::vector<std::unique_ptr<AsyncProcessor>> async_;
};

class FileDescriptorWrapper : public QObject {
  Q_OBJECT

 public:
  FileDescriptorWrapper(const google::protobuf::FileDescriptor* descriptor,
                        QObject* p = 0)
      : QObject(p), descriptor_(descriptor) {}

  Q_INVOKABLE qpb::DescriptorWrapper* messageType(int i) {
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
  Q_INVOKABLE qpb::FileDescriptorWrapper* addFileDescriptor(QVariant);

 private:
  std::vector<QByteArray> encoded_descriptors_;
  google::protobuf::DescriptorPool pool_;
};
}
