#ifndef PROTOBUF_QML_PROCESSOR_H
#define PROTOBUF_QML_PROCESSOR_H

#include "descriptor_database.h"
#include <QObject>
#include <QMetaType>

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
    if (!input) {
      error(async_id, "input is null");
      return;
    }
    if (!descriptor) {
      error(async_id, "descriptor is null.");
      return;
    }
    auto x = descriptor->parse(input);
    qDebug() << "parsed";
    if (!x.isValid() || !x.canConvert(QMetaType::QVariantList)) {
      error(async_id, "Failed to parse any message");
    } else {
      parsed(async_id, x.value<QVariantList>());
    }
  }

  Q_INVOKABLE void serialize(OutputDevice* output,
                             DescriptorWrapper* descriptor,
                             const QVariantList& fields,
                             const QList<int>& oneofs,
                             int async_id) {
    qDebug() << "serialize";
    if (!output) {
      error(async_id, "input is null");
      return;
    }
    if (!descriptor) {
      error(async_id, "descriptor is null.");
      return;
    }
    if (!descriptor->serialize(output, fields, oneofs)) {
      error(async_id, "failed to serialize");
    } else {
      qDebug() << "serialized";
      serialized(async_id);
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
