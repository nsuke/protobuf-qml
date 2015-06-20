#include "protobuf/qml/qml_generator.h"
#include "protobuf/qml/compiler_util.h"

#include <google/protobuf/any.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <iostream>
#include <memory>

using namespace protobuf::qml;
using namespace google::protobuf;

int main() {
  auto file = google::protobuf::Any::descriptor()->file();
  auto path = "test_tmp";

  std::unique_ptr<io::ZeroCopyOutputStream> out(
      new io::OstreamOutputStream(&std::cout));
  io::Printer p(out.get(), '$');
  FileGenerator g(file);
  g.generateJsFile(p);
  return 0;
}
