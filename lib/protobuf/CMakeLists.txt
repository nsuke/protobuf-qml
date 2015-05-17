
set(protobuf-qml_SOURCES
  qml/descriptors.cpp
  qml/file.cpp
  qml/memory.cpp
  qml/protobuf_plugin.cpp
  qml/processor.cpp)
add_library(protobuf-qml SHARED ${protobuf-qml_SOURCES})
protobuf_qml_target(protobuf-qml)
target_compile_definitions(protobuf-qml PRIVATE -DPROTOBUF_QML_EXPORT)
target_link_libraries(protobuf-qml
  ${PROTOBUF_LIBRARIES}
  Qt5::Qml)

copy(protobuf-qml
  ${CMAKE_BINARY_DIR}/plugins/Protobuf
  imports/qmldir
  imports/call.js
  imports/message.js
  imports/BufferService.qml
  imports/MemoryBuffer.qml
  imports/File.qml
  ${CMAKE_CURRENT_BINARY_DIR}/libprotobuf-qml.so)
