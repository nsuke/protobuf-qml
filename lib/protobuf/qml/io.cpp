#include "protobuf/qml/io.h"
#include <google/protobuf/io/zero_copy_stream.h>

namespace protobuf {
namespace qml {

InputDevice::SessionPtr InputDevice::createSession() {
  return SessionPtr();
}

OutputDevice::SessionPtr OutputDevice::createSession() {
  return SessionPtr();
}
}
}
