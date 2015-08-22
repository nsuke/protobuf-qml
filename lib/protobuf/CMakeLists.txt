#
# core library
#
set(protobuf-qml-core_SOURCES
  qml/descriptors.cpp
  qml/file.cpp
  qml/memory.cpp
  qml/buffer.cpp
  qml/method.cpp
  qml/server_method.cpp
  qml/v4/descriptor.cpp
  qml/v4/v4util.cpp
  )

add_library(protobuf-qml-core SHARED ${protobuf-qml-core_SOURCES})
protobuf_qml_target(protobuf-qml-core)

target_compile_definitions(protobuf-qml-core PRIVATE PROTOBUF_QML_EXPORT)
set_target_properties(
  protobuf-qml-core
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(protobuf-qml-core
  ${PROTOBUF_LIBRARIES}
  Qt5::Quick)

#
# QML plugin
#
set(protobuf-qml_SOURCES
  qml/protobuf_plugin.cpp)

add_library(protobuf-qml SHARED ${protobuf-qml_SOURCES})
protobuf_qml_target(protobuf-qml)

set_target_properties(
  protobuf-qml
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Protobuf
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Protobuf)

target_link_libraries(protobuf-qml
  protobuf-qml-core
  ${PROTOBUF_LIBRARIES}
  Qt5::Qml)

copy(protobuf-qml
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Protobuf
  imports/qmldir
  imports/message.js
  imports/errors.js
  imports/UnaryMethod.qml
  imports/ClientMethod.qml
  imports/WriterMethod.qml
  imports/ReaderMethod.qml
  imports/ReaderWriterMethod.qml
  imports/ServerUnaryMethod.qml
  imports/ServerReaderMethod.qml
  imports/ServerWriterMethod.qml
  imports/ServerReaderWriterMethod.qml
  imports/BufferService.qml
  imports/MemoryBuffer.qml
  imports/File.qml)
