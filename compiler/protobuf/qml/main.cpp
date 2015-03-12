#include "protobuf/qml/qml_generator.h"

#include <google/protobuf/compiler/plugin.h>

int main(int argc, char* argv[]) {
  protobuf::qml::QmlGenerator gen;
  return google::protobuf::compiler::PluginMain(argc, argv, &gen);
}
