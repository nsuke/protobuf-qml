#ifndef PROTOBUF_QML_PROCESSOR_H
#define PROTOBUF_QML_PROCESSOR_H

#include "protobuf/qml/descriptors.h"

#include <QDebug>
#include <QMetaType>
#include <QObject>

namespace google {
namespace protobuf {
namespace io {
class ZeroCopyInputStream;
class ZeroCopyOutputStream;
}
}
}

namespace protobuf {
namespace qml {

class Processor : public QObject {
  Q_OBJECT
  Q_PROPERTY(protobuf::qml::DescriptorWrapper* readDescriptor READ
                 read_descriptor WRITE set_read_descriptor NOTIFY
                     readDescriptorChanged)
  Q_PROPERTY(protobuf::qml::DescriptorWrapper* writeDescriptor READ
                 write_descriptor WRITE set_write_descriptor NOTIFY
                     writeDescriptorChanged)

signals:
  void readDescriptorChanged();
  void writeDescriptorChanged();
  void data(int tag, const QVariant& data);
  void dataEnd(int tag);
  void error(int tag, const QVariant& data);

public:
  Processor(QObject* p = 0) : QObject(p) {}
  virtual ~Processor() {}

  DescriptorWrapper* read_descriptor() const { return read_desc_; }
  void set_read_descriptor(DescriptorWrapper* read_desc) {
    if (read_desc != read_desc_) {
      read_desc_ = read_desc;
      readDescriptorChanged();
    }
  }

  DescriptorWrapper* write_descriptor() const { return write_desc_; }
  void set_write_descriptor(DescriptorWrapper* write_desc) {
    if (write_desc != write_desc_) {
      write_desc_ = write_desc;
      writeDescriptorChanged();
    }
  }

  // returns seemingly unused tag, not guaranteed.
  Q_INVOKABLE int getFreeTag() { return ++max_tag_; }

  // TODO: need timeout
  Q_INVOKABLE void write(int tag, const QVariant& data);

  Q_INVOKABLE void writeEnd(int tag) {
    max_tag_ = std::max(tag, max_tag_);
    if (!write_desc_) {
      qWarning("Descriptor is null");
      error(tag, "Descriptor is null.");
      return;
    }
    doWriteEnd(tag);
  }

protected:
  virtual void doWrite(int tag, const google::protobuf::Message& msg) {
    error(tag, "Not supported.");
  }
  virtual void doEmptyWrite(int tag) {
    error(tag, "Cannot write empty message");
  }

  virtual void doWriteEnd(int tag) { error(tag, "Not supported."); }

  void emitMessageData(int tag, const google::protobuf::Message& msg) {
    if (!read_desc_) {
      qWarning() << "Descriptor is null.";
      // TODO: need to emit error here ?
      return;
    }
    data(tag, read_desc_->dataFromMessage(msg));
  }

private:
  int max_tag_ = 0;
  DescriptorWrapper* read_desc_ = nullptr;
  DescriptorWrapper* write_desc_ = nullptr;
};

class ProtobufSerializer : public Processor {
  Q_OBJECT

public:
  explicit ProtobufSerializer(QObject* p = nullptr) : Processor(p) {}

protected:
  void doWrite(int tag, const google::protobuf::Message& msg) final;

  virtual google::protobuf::io::ZeroCopyOutputStream* open(int tag,
                                                           int hint) = 0;
  virtual void close(int tag,
                     google::protobuf::io::ZeroCopyOutputStream* stream);
};

class ProtobufParser : public Processor {
  Q_OBJECT

public:
  explicit ProtobufParser(QObject* p = nullptr) : Processor(p) {}

protected:
  void doWrite(int tag, const google::protobuf::Message& msg) final;
  void doEmptyWrite(int tag) final;

  virtual google::protobuf::io::ZeroCopyInputStream* open(int tag) = 0;
  virtual void close(int tag,
                     google::protobuf::io::ZeroCopyInputStream* stream);
};

class GenericStreamProcessor;

namespace detail {
class GenericSerializer : public ProtobufSerializer {
  Q_OBJECT
public:
  GenericSerializer(GenericStreamProcessor* p);

protected:
  google::protobuf::io::ZeroCopyOutputStream* open(int tag, int hint) final;
  void close(int tag, google::protobuf::io::ZeroCopyOutputStream* stream) final;

private:
  GenericStreamProcessor* p_;
};

class GenericParser : public ProtobufParser {
  Q_OBJECT
public:
  GenericParser(GenericStreamProcessor* p);

protected:
  google::protobuf::io::ZeroCopyInputStream* open(int tag) final;
  void close(int tag, google::protobuf::io::ZeroCopyInputStream* stream) final;

private:
  GenericStreamProcessor* p_;
};
}

class GenericStreamProcessor : public QObject {
  Q_OBJECT

  Q_PROPERTY(protobuf::qml::Processor* input READ input);
  Q_PROPERTY(protobuf::qml::Processor* output READ output);

public:
  GenericStreamProcessor(QObject* p = 0);

  Processor* input();
  Processor* output();

protected:
  virtual google::protobuf::io::ZeroCopyInputStream* openInput(int tag) = 0;
  virtual google::protobuf::io::ZeroCopyOutputStream* openOutput(int tag,
                                                                 int hint) = 0;
  virtual void closeOutput(int tag,
                           google::protobuf::io::ZeroCopyOutputStream* stream) {
  }
  virtual void closeInput(int tag,
                          google::protobuf::io::ZeroCopyInputStream* stream) {}
  void parseError(int tag, const QVariant& data) { parser_->error(tag, data); }
  void serializeError(int tag, const QVariant& data) { ser_->error(tag, data); }

private:
  std::unique_ptr<detail::GenericSerializer> ser_;
  std::unique_ptr<detail::GenericParser> parser_;

  friend class detail::GenericSerializer;
  friend class detail::GenericParser;
};
}
}

#endif  // PROTOBUF_QML_PROCESSOR_H
