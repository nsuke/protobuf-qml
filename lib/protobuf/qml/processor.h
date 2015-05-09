#ifndef PROTOBUF_QML_PROCESSOR_H
#define PROTOBUF_QML_PROCESSOR_H

#include "descriptors.h"
#include <QDebug>
#include <QMetaType>
#include <QObject>

namespace protobuf {
namespace qml {

class Processor : public QObject {
  Q_OBJECT
  Q_PROPERTY(protobuf::qml::DescriptorWrapper* descriptor READ descriptor WRITE set_descriptor
                 NOTIFY descriptorChanged)

signals:
  void descriptorChanged();
  void data(int tag, const QVariant& data);
  void dataEnd(int tag);
  void error(int tag, const QVariant& data);

 public:
  Processor(QObject* p = 0) : QObject(p) {}
  virtual ~Processor() {}
  DescriptorWrapper* descriptor() const { return desc_; }
  void set_descriptor(DescriptorWrapper* desc) {
    if (desc != desc_) {
      desc_ = desc;
      descriptorChanged();
    }
  }

  // returns seemingly unused tag, not guaranteed.
  Q_INVOKABLE int getFreeTag() {
    return ++max_tag_;
  }

  // TODO: need timeout
  Q_INVOKABLE void write(int tag, const QVariant& data);

  Q_INVOKABLE void writeEnd(int tag) {
    max_tag_ = std::max(tag, max_tag_);
    if (!desc_) {
      error(tag, "Descriptor is null.");
      return;
    }
    doWriteEnd(tag);
  }

  Q_INVOKABLE void read(int tag) {
    max_tag_ = std::max(tag, max_tag_);
    if (!desc_) {
      error(tag, "Descriptor is null.");
      return;
    }
    doRead(tag);
  }

 protected:
  virtual void doWrite(int tag, const google::protobuf::Message& msg) {
    error(tag, "Not supported.");
  }

  virtual void doWriteEnd(int tag) {
    error(tag, "Not supported.");
  }

  virtual void doRead(int tag) {
    error(tag, "Not supported.");
  }

  void emitMessageData(int tag, const google::protobuf::Message& msg) {
    if (!desc_) {
      qWarning() << "Descriptor is null.";
      return;
    }
    data(tag, desc_->dataFromMessage(msg));
  }

 private:
  int max_tag_ = 0;
  DescriptorWrapper* desc_ = nullptr;
};

}
}

#endif  // PROTOBUF_QML_PROCESSOR_H
