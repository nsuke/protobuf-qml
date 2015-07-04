#
# gRPC QML plugin
#
set(grpc-qml_SOURCES
  qml/server_unary.cpp
  qml/server_reader.cpp
  qml/server_writer.cpp
  qml/server.cpp
  qml/server_credentials.cpp
  qml/credentials.cpp
  qml/grpc_plugin.cpp
  qml/base.cpp
  qml/unary.cpp
  qml/writer.cpp
  qml/reader.cpp
  )

add_library(grpc-qml SHARED ${grpc-qml_SOURCES})
protobuf_qml_target(grpc-qml)

set_target_properties(
  grpc-qml
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/Grpc)

target_link_libraries(grpc-qml
  protobuf-qml-core
  ${GRPC++_LIBRARIES}
  Qt5::Qml)

copy(grpc-qml
  ${CMAKE_BINARY_DIR}/plugins/Grpc
  imports/qmldir)

add_subdirectory(test)
