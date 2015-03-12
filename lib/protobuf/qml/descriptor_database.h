#include "protobuf/qml/io.h"
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

class AsyncProcessor : public QObject {
  Q_OBJECT

 signals:
  void startParse(int key, InputDevice* input);
  void startSerialize(int key, OutputDevice* output, QVariantMap value);
  void clearSharedMessage();

 public:
  AsyncProcessor(DescriptorWrapper* parent)
      : QObject(reinterpret_cast<QObject*>(parent)), parent_(parent) {
    moveToThread(&thread_);
    connect(this, &AsyncProcessor::startParse, this, &AsyncProcessor::doParse);
    connect(this,
            &AsyncProcessor::startSerialize,
            this,
            &AsyncProcessor::doSerialize);
    connect(this,
            &AsyncProcessor::clearSharedMessage,
            this,
            &AsyncProcessor::doClearSharedMessage);
    thread_.start();
  }
  ~AsyncProcessor();

  bool has_task() const { return has_task_; }

 private slots:
  void doParse(int key, InputDevice* input);
  void doSerialize(int key, OutputDevice* input, QVariantMap value);
  void doClearSharedMessage();

 private:
  bool has_task_ = false;
  DescriptorWrapper* parent_;
  QThread thread_;
};

class DescriptorWrapper : public QObject {
  Q_OBJECT
  Q_PROPERTY(int maxThreads READ max_threads WRITE set_max_threads NOTIFY
                 maxThreadsChanged)

 signals:
  void parseCompleted(int key, QVariant result, bool error);
  void serializeCompleted(int key, bool error);
  void maxThreadsChanged();

 public:
  DescriptorWrapper(const google::protobuf::Descriptor* descriptor,
                    QObject* p = 0)
      : QObject(p), descriptor_(descriptor) {}
  ~DescriptorWrapper();

  Q_INVOKABLE QVariant parse(InputDevice* input);
  Q_INVOKABLE bool serialize(OutputDevice* output, QVariantMap value);

  Q_INVOKABLE int nextKey() { return ++key_; }
  Q_INVOKABLE bool parseAsync(int key, InputDevice* input) {
    auto index = async();
    if (index < 0) return 0;
    async_[index]->startParse(key, input);
    return true;
  }
  Q_INVOKABLE bool serializeAsync(int key,
                                  OutputDevice* output,
                                  QVariantMap value) {
    auto index = async();
    if (index < 0) return false;
    async_[index]->startSerialize(key, output, std::move(value));
    return true;
  }

  int max_threads() const { return max_threads_; }
  void set_max_threads(int value) {
    if (max_threads_ != value) {
      max_threads_ = value;
      maxThreadsChanged();
    };
  }

  void clearSharedMessage() { message_.setLocalData(nullptr); }

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
        return -1;
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
  std::vector<AsyncProcessor*> async_;
};

class FileDescriptorWrapper : public QObject {
  Q_OBJECT

 public:
  FileDescriptorWrapper(const google::protobuf::FileDescriptor* descriptor,
                        QObject* p = 0)
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
