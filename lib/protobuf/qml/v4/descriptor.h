#ifndef PROTOBUF_QML_V4_DESCRIPTOR_H
#define PROTOBUF_QML_V4_DESCRIPTOR_H

#include "protobuf/qml/common.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <QObject>
#include <QRunnable>
#include <private/qv8engine_p.h>
#include <memory>

// #if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
#include "private/qv4persistent_p.h"
// #endif

namespace protobuf {
namespace qml {

class PROTOBUF_QML_DLLEXPORT Descriptor : public QObject {
  Q_OBJECT

public:
  explicit Descriptor(const google::protobuf::Descriptor* descriptor,
                      QObject* p = nullptr)
      : QObject(p), descriptor_(descriptor) {}

  Q_INVOKABLE void serialize(QQmlV4Function*);

  Q_INVOKABLE void parse(QQmlV4Function*);

  std::unique_ptr<google::protobuf::Message> jsValueToMessage(
      QV4::ExecutionEngine*, QV4::ArrayObject&);

  bool jsValueToMessage(QV4::ExecutionEngine*,
                        QV4::ArrayObject&,
                        google::protobuf::Message&);

  QV4::ReturnedValue messageToJsValue(QV4::ExecutionEngine*,
                                      const google::protobuf::Message&);

  google::protobuf::Message* parseToNewMessage(const char* data, int size);
  google::protobuf::Message* newMessage() {
    return message_factory_.GetPrototype(descriptor_)->New();
  }

  google::protobuf::Message* defaultMessage() {
    if (!default_message_) {
      default_message_.reset(message_factory_.GetPrototype(descriptor_)->New());
    }
    return default_message_.get();
  }

private:
  QV4::ReturnedValue getFieldValue(
      QV4::ExecutionEngine*,
      const google::protobuf::Reflection& ref,
      const google::protobuf::Message& msg,
      const google::protobuf::FieldDescriptor* field);

  QV4::ReturnedValue getRepeatedFieldValue(
      QV4::ExecutionEngine*,
      const google::protobuf::Reflection& ref,
      const google::protobuf::Message& msg,
      const google::protobuf::FieldDescriptor* field,
      int size);

  void setFieldValue(QV4::ExecutionEngine*,
                     const google::protobuf::Reflection& ref,
                     google::protobuf::Message& msg,
                     const google::protobuf::FieldDescriptor* field,
                     QV4::ReturnedValue value);

  void setRepeatedFieldValue(QV4::ExecutionEngine*,
                             const google::protobuf::Reflection& ref,
                             google::protobuf::Message& msg,
                             const google::protobuf::FieldDescriptor* field,
                             QV4::ReturnedValue list);

  const google::protobuf::Descriptor* descriptor_;
  google::protobuf::DynamicMessageFactory message_factory_;
  std::unique_ptr<google::protobuf::Message> default_message_;
};

class SerializeTask : public QObject, public QRunnable {
  Q_OBJECT

signals:
  void done(const QByteArray&);

public:
  SerializeTask(std::unique_ptr<google::protobuf::Message> msg,
                QV4::ExecutionEngine* v4,
                const QV4::Value& callback);

  void run() final;

private slots:
  void onDone(const QByteArray& ba);

private:
  std::unique_ptr<google::protobuf::Message> msg_;
  QV4::ExecutionEngine* v4_;
  QV4::PersistentValue callback_;
};

class ParseTask : public QObject, public QRunnable {
  Q_OBJECT

signals:
  void done();

public:
  ParseTask(Descriptor* p,
            google::protobuf::Message* msg,
            QByteArray buf,
            QV4::ExecutionEngine* v4,
            const QV4::Value& callback);

  void run() final;

private slots:
  void onDone();

private:
  Descriptor* p_;
  std::unique_ptr<google::protobuf::Message> msg_;
  QByteArray buf_;
  QV4::ExecutionEngine* v4_;
  QV4::PersistentValue callback_;
};
}
}

#endif  // PROTOBUF_QML_V4_DESCRIPTOR_H
