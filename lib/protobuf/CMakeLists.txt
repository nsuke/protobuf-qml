#
# core library
#
set(protobuf-qml-core_SOURCES
  qml/descriptors.cpp
  qml/file.cpp
  qml/memory.cpp
  qml/method.cpp
  qml/processor.cpp)

add_library(protobuf-qml-core SHARED ${protobuf-qml-core_SOURCES})
protobuf_qml_target(protobuf-qml-core)

set_target_properties(
  protobuf-qml-core
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(protobuf-qml-core
  ${PROTOBUF_LIBRARIES}
  Qt5::Qml)

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
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/Protobuf)

target_compile_definitions(protobuf-qml PRIVATE -DPROTOBUF_QML_EXPORT)

target_link_libraries(protobuf-qml
  protobuf-qml-core
  ${PROTOBUF_LIBRARIES}
  Qt5::Qml)

copy(protobuf-qml
  ${CMAKE_BINARY_DIR}/plugins/Protobuf
  imports/qmldir
  imports/call.js
  imports/message.js
  imports/UnaryMethod.qml
  imports/WriterMethod.qml
  imports/BufferService.qml
  imports/MemoryBuffer.qml
  imports/File.qml)
