#include "protobuf/qml/processor.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

void Processor::write(int tag, const QVariant& data) {
  max_tag_ = std::max(tag, max_tag_);
  if (data.isValid()) {
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
  } else {
    doEmptyWrite(tag);
  }
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

GenericSerializer::GenericSerializer(GenericStreamProcessor* p)
    : ProtobufSerializer(p), p_(p) {
}
google::protobuf::io::ZeroCopyOutputStream* GenericSerializer::open(int tag,
                                                                    int hint) {
  return p_->openOutput(tag, hint);
}
void GenericSerializer::close(
    int tag, google::protobuf::io::ZeroCopyOutputStream* stream) {
  p_->closeOutput(tag, stream);
}

GenericParser::GenericParser(GenericStreamProcessor* p)
    : ProtobufParser(p), p_(p) {
}

google::protobuf::io::ZeroCopyInputStream* GenericParser::open(int tag) {
  return p_->openInput(tag);
}

void GenericParser::close(int tag,
                          google::protobuf::io::ZeroCopyInputStream* stream) {
  p_->closeInput(tag, stream);
}
}

GenericStreamProcessor::GenericStreamProcessor(QObject* p)
    : QObject(p),
      ser_(new detail::GenericSerializer(this)),
      parser_(new detail::GenericParser(this)) {
}

Q_INVOKABLE Processor* GenericStreamProcessor::input() {
  return parser_.get();
}

Q_INVOKABLE Processor* GenericStreamProcessor::output() {
  return ser_.get();
}
}
}
