#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/util.h"
#include <google/protobuf/descriptor.pb.h>

#include <QDebug>
#include <QVariantList>
#include <sstream>
#include <vector>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

FileDescriptorWrapper* DescriptorPoolWrapper::addFileDescriptor(
    QVariant encoded) {
  if (!encoded.canConvert<QByteArray>()) {
    return nullptr;
  }
  auto ba = QByteArray::fromBase64(encoded.value<QByteArray>());

  FileDescriptorProto pb;
  if (pb.ParseFromArray(ba.data(), ba.size())) {
    if (auto desc = pool_.BuildFile(pb)) {
      auto file = new FileDescriptorWrapper(desc);
      children_.emplace_back(file);
      return file;
    }
  }
  return nullptr;
}
}
}
