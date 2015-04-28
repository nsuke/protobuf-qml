#ifndef PROTOBUF_QML_PROCESSOR_H
#define PROTOBUF_QML_PROCESSOR_H

#include "descriptors.h"
#include <QObject>
#include <QMetaType>

namespace protobuf {
namespace qml {

class AsyncProcessor : public QObject {
  Q_OBJECT

 signals:
  void parsed(int id, QVariantList fields, const QVariant& err);
  void serialized(int id, const QVariant& err);
  void serializedArray(int id, QByteArray data, const QVariant& err);

 public:
  Q_INVOKABLE void parseArray(const QByteArray& array,
                              DescriptorWrapper* descriptor,
                              int async_id) {
    qDebug() << "parseArray";
    raiseParseError(async_id, notImplemented());
  }

  Q_INVOKABLE void parse(InputDevice* input,
                         DescriptorWrapper* descriptor,
                         int async_id) {
    qDebug() << "parse";
    if (!input) {
      raiseParseError(async_id, invalidArg("input is null"));
      return;
    }
    if (!descriptor) {
      raiseParseError(async_id, invalidArg("descriptor is null"));
      return;
    }
    auto x = descriptor->parse(input);
    qDebug() << "parsed";
    if (!x.isValid() || !x.canConvert(QMetaType::QVariantList)) {
      raiseParseError(async_id, parseError("Failed to parse any message"));
    } else {
      parsed(async_id, x.value<QVariantList>(), noError());
    }
  }

  Q_INVOKABLE void serialize(OutputDevice* output,
                             DescriptorWrapper* descriptor,
                             const QVariantList& fields,
                             const QList<int>& oneofs,
                             int async_id) {
    qDebug() << "serialize";
    if (!output) {
      raiseSerializationError(async_id, invalidArg("input is null"));
      return;
    }
    if (!descriptor) {
      raiseSerializationError(async_id, invalidArg("descriptor is null"));
      return;
    }
    if (!descriptor->serialize(output, fields, oneofs)) {
      raiseSerializationError(async_id, serializationError());
    } else {
      qDebug() << "serialized";
      serialized(async_id, noError());
    }
  }

  Q_INVOKABLE void serializeArray(DescriptorWrapper* descriptor,
                                  const QVariantList& fields,
                                  int async_id) {
    qDebug() << "serializeArray";
    raiseSerializeArrayError(async_id, notImplemented());
  }

 private:
  QVariant error(const QString& name, const QString& message) {
    return QVariantMap{{"name", name}, {"message", message}};
  }

  QVariant noError() { return QVariant(); }

  QVariant parseError(const QString& message) {
    return error("ParseError", message);
  }

  void raiseParseError(int id, QVariant err) {
    qWarning() << "raiseParseError : " << err;
    parsed(id, QVariantList(), err);
  }

  void raiseSerializationError(int id, QVariant err) {
    serialized(id, err);
  }

  void raiseSerializeArrayError(int id, QVariant err) {
    serializedArray(id, QByteArray(), err);
  }

  QVariant serializationError(const QString& message = "") {
    return error("SerializationError", message);
  }

  QVariant notImplemented() {
    return error("NotImplementedError", "Not implemented");
  }

  QVariant invalidArg(const QString& message) {
    return error("ArgumentError", message);
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
