#include "qpb/io.h"
#include <google/protobuf/io/zero_copy_stream.h>

namespace qpb {

InputDevice::SessionPtr InputDevice::createSession() {
  return SessionPtr();
}

OutputDevice::SessionPtr OutputDevice::createSession() {
  return SessionPtr();
}
}
