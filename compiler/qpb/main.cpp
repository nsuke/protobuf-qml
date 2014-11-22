#include "qpb/qml_generator.h"

#include <google/protobuf/compiler/plugin.h>

int main(int argc, char* argv[]) {
  qpb::QmlGenerator gen;
  return google::protobuf::compiler::PluginMain(argc, argv, &gen);
}
