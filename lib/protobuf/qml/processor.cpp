#include "protobuf/qml/processor.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

namespace detail {
Worker::Worker(Processor* p) {
  moveToThread(&thread_);
  connect(this, &Worker::write,
          [p](int tag, const QVariant& data) { p->writeThread(tag, data); });
  connect(this, &Worker::writeEmpty,
          [p](int tag) { p->writeEmptyThread(tag); });
  connect(this, &Worker::writeEnd, [p](int tag) { p->writeEndThread(tag); });
  thread_.start();
}
}

void Processor::write(int tag, const QVariant& data) {
  max_tag_ = std::max(tag, max_tag_);
  if (data.isValid()) {
    if (!write_desc_) {
      qWarning("Descriptor is null");
      error(tag, "Descriptor is null.");
      return;
    }
    if (async_) {
      ensureWorker();
      worker_->write(tag, data);
    } else {
      writeThread(tag, data);
    }
  } else {
    if (async_) {
      ensureWorker();
      worker_->writeEmpty(tag);
    } else {
      writeEmptyThread(tag);
    }
  }
}

void Processor::writeThread(int tag, const QVariant& data) {
  if (!write_desc_) {
    qWarning("Descriptor is null");
    error(tag, "Descriptor is null.");
    return;
  }
  auto msg = write_desc_->dataToMessage(data);
  if (!msg) {
    error(tag, "Failed to convert message data to protobuf message object.");
    return;
  }
  doWrite(tag, *msg);
}

void ProtobufParser::doWrite(int tag, const google::protobuf::Message& empty) {
  qWarning("Parser received non-empty argument.");
  error(tag, "Parser received non-empty argument.");
}
void ProtobufParser::doEmptyWrite(int tag) {
  auto close =
      std::bind(&ProtobufParser::close, this, tag, std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyInputStream, decltype(close)> stream(open(tag),
                                                                   close);
  if (!stream) {
    error(tag, "Failed to open input stream");
    return;
  }
  std::unique_ptr<google::protobuf::Message> msg(
      read_descriptor()->newMessage());
  if (!msg) {
    error(tag, "Failed to create protobuf message object");
    return;
  }
  if (!msg->ParseFromZeroCopyStream(stream.get())) {
    error(tag, "Failed to parse from protobuf buffer");
    return;
  }
  stream.reset();
  emitMessageData(tag, *msg);
}

void ProtobufSerializer::doWrite(int tag,
                                 const google::protobuf::Message& msg) {
  auto close =
      std::bind(&ProtobufSerializer::close, this, tag, std::placeholders::_1);
  std::unique_ptr<io::ZeroCopyOutputStream, decltype(close)> stream(
      open(tag, msg.ByteSize()), close);
  if (!stream) {
    error(tag, "Failed to open output stream");
    return;
  }
  if (!msg.SerializeToZeroCopyStream(stream.get())) {
    error(tag, "Failed to serialize to protobuf stream");
    return;
  }
  // Flush before notifying "done".
  stream.reset();
  data(tag, QVariant());
}

void ProtobufParser::close(int tag,
                           google::protobuf::io::ZeroCopyInputStream* stream) {
  if (stream) delete stream;
}
void ProtobufSerializer::close(
    int tag, google::protobuf::io::ZeroCopyOutputStream* stream) {
  if (stream) delete stream;
}

namespace detail {

GenericSerializer::GenericSerializer(GenericChannel* channel, QObject* p)
    : ProtobufSerializer(p), channel_(channel) {
  connect(channel, &GenericChannel::serializeError, this,
          &GenericSerializer::error);
}
google::protobuf::io::ZeroCopyOutputStream* GenericSerializer::open(int tag,
                                                                    int hint) {
  return channel_->openOutput(tag, hint);
}
void GenericSerializer::close(
    int tag, google::protobuf::io::ZeroCopyOutputStream* stream) {
  channel_->closeOutput(tag, stream);
}

GenericParser::GenericParser(GenericChannel* channel, QObject* p)
    : ProtobufParser(p), channel_(channel) {
  connect(channel, &GenericChannel::parseError, this, &GenericParser::error);
}

google::protobuf::io::ZeroCopyInputStream* GenericParser::open(int tag) {
  return channel_->openInput(tag);
}

void GenericParser::close(int tag,
                          google::protobuf::io::ZeroCopyInputStream* stream) {
  channel_->closeInput(tag, stream);
}
}

GenericStreamProcessor::GenericStreamProcessor(QObject* p) : QObject(p) {
}

void GenericStreamProcessor::set_channel(GenericChannel* channel) {
  if (channel_ != channel) {
    channel_ = channel;
    if (channel_) {
      ser_.reset(new detail::GenericSerializer(channel, this));
      parser_.reset(new detail::GenericParser(channel, this));
    } else {
      ser_.reset();
      parser_.reset();
    }
    channelChanged();
    inputChanged();
    outputChanged();
  }
}

Q_INVOKABLE Processor* GenericStreamProcessor::input() {
  return parser_.get();
}

Q_INVOKABLE Processor* GenericStreamProcessor::output() {
  return ser_.get();
}
}
}
