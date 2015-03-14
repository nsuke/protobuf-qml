#include "protobuf/qml/processor.h"

namespace protobuf {
namespace qml {
}}

/*
AsyncProcessor::~AsyncProcessor() {
  thread_.quit();
  thread_.wait();
}

void AsyncProcessor::doClearSharedMessage() {
  parent_->clearSharedMessage();
}

void AsyncProcessor::doParse(int key, InputDevice* input) {
  Q_ASSERT(!has_task_);
  has_task_ = true;
  auto msg = parent_->parse(input);
  auto err = !msg.isValid();
  parent_->parseCompleted(key, std::move(msg), err);
  has_task_ = false;
}

void AsyncProcessor::doSerialize(int key,
                                 OutputDevice* output,
                                 QVariantMap value) {
  Q_ASSERT(!has_task_);
  has_task_ = true;
  auto res = parent_->serialize(output, value);
  parent_->serializeCompleted(key, !res);
  has_task_ = false;
}
*/
