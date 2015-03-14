#ifndef PROTOBUF_QML_PROCESSOR_H
#define PROTOBUF_QML_PROCESSOR_H

#include "descriptor_database.h"
#include <QObject>

namespace protobuf {
namespace qml {

class AsyncProcessor : public QObject {
  Q_OBJECT

 signals:
  void error(int id, const QString& error);
  void parsed(int id, QVariantList fields);
  void serialized(int id);
  void serializedArray(int id, QByteArray data);

 public:
  Q_INVOKABLE void parseArray(const QByteArray& array,
                              DescriptorWrapper* descriptor,
                              int async_id) {
    qDebug() << "parseArray";
    error(async_id, "Not implemented");
  }

  Q_INVOKABLE void parse(InputDevice* input,
                         DescriptorWrapper* descriptor,
                         int async_id) {
    qDebug() << "parse";
    if (descriptor) {
      auto x = descriptor->parse(input);
      qDebug() << "parsed";
      parsed(async_id, x);
    } else {
      qWarning() <<  "Descriptor is null.";
      error(async_id, "Descriptor is null.");
    }
  }

  Q_INVOKABLE void serialize(OutputDevice* output,
                             DescriptorWrapper* descriptor,
                             const QVariantList& fields,
                             int async_id) {
    qDebug() << "serialize";
    if (descriptor) {
      descriptor->serialize(output, fields);
      qDebug() << "serialized";
      serialized(async_id);
    } else {
      qWarning() <<  "Descriptor is null.";
      error(async_id, "Descriptor is null.");
    }
  }

  Q_INVOKABLE void serializeArray(DescriptorWrapper* descriptor,
                                  const QVariantList& fields,
                                  int async_id) {
    qDebug() << "serializeArray";
    error(async_id, "Not implemented");
  }

  // signals:
  //  void startParse(int key, InputDevice* input);
  //  void startSerialize(int key, OutputDevice* output, QVariantMap value);
  //  void clearSharedMessage();
  //
  // public:
  //  AsyncProcessor(DescriptorWrapper* parent)
  //      : QObject(reinterpret_cast<QObject*>(parent)), parent_(parent) {
  //    moveToThread(&thread_);
  //    connect(this, &AsyncProcessor::startParse, this,
  //    &AsyncProcessor::doParse);
  //    connect(this,
  //            &AsyncProcessor::startSerialize,
  //            this,
  //            &AsyncProcessor::doSerialize);
  //    connect(this,
  //            &AsyncProcessor::clearSharedMessage,
  //            this,
  //            &AsyncProcessor::doClearSharedMessage);
  //    thread_.start();
  //  }
  //  ~AsyncProcessor();
  //
  //  bool has_task() const { return has_task_; }
  //
  // private slots:
  //  void doParse(int key, InputDevice* input);
  //  void doSerialize(int key, OutputDevice* input, QVariantMap value);
  //  void doClearSharedMessage();
  //
  // private:
  //  bool has_task_ = false;
  //  DescriptorWrapper* parent_;
  //  QThread thread_;
};
}
}

#endif  // PROTOBUF_QML_PROCESSOR_H
