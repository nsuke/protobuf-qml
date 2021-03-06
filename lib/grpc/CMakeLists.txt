#
# gRPC QML plugin
#
set(grpc-qml_SOURCES
  qml/logging.cpp
  qml/server_unary.cpp
  qml/server_reader.cpp
  qml/server_writer.cpp
  qml/server_bidi.cpp
  qml/server.cpp
  qml/server_credentials.cpp
  qml/credentials.cpp
  qml/grpc_plugin.cpp
  qml/base.cpp
  qml/unary.cpp
  qml/writer.cpp
  qml/reader.cpp
  qml/reader_writer.cpp
  )

add_library(grpc-qml SHARED ${grpc-qml_SOURCES})
protobuf_qml_target(grpc-qml)

target_compile_definitions(grpc-qml PRIVATE GRPC_QML_EXPORT)
set_target_properties(
  grpc-qml
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Grpc
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Grpc)

target_link_libraries(grpc-qml PUBLIC
  protobuf-qml-core
  ${GRPC++_LIBRARIES}
  Qt5::Qml)

copy(grpc-qml
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Grpc
  imports/qmldir)
