#
# protobuf compiler plugin for QML/JS
#
set(protoc-gen-qml-core_SOURCES
  protobuf/qml/util.cpp
  protobuf/qml/enum_generator.cpp
  protobuf/qml/field_generator.cpp
  protobuf/qml/message_generator.cpp
  protobuf/qml/oneof_generator.cpp
  protobuf/qml/service_generator.cpp
  protobuf/qml/qml_generator.cpp
  protobuf/qml/compiler_util.cpp)
option(PROTOBUF_COMPILER_HAS_QT "Allow compiler plugin dependency on Qt" OFF)
if(NOT PROTOBUF_COMPILER_HAS_QT)
  list(APPEND protoc-gen-qml-core_SOURCES protobuf/qml/base64.cpp)
endif()
add_library(protoc-gen-qml-core
  ${protoc-gen-qml-core_SOURCES})
target_link_libraries(protoc-gen-qml-core
  protobuf-qml
  ${PROTOBUF_PROTOC_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  Qt5::Core)

set(protoc-gen-qml_SOURCES
  protobuf/qml/main.cpp)
add_executable(protoc-gen-qml ${protoc-gen-qml_SOURCES})
protobuf_qml_target(protoc-gen-qml)

option(PROTOBUF_COMPILER_HAS_QT "Allow compiler plugin dependency on Qt" OFF)
if(PROTOBUF_COMPILER_HAS_QT)
  target_compile_definitions(protoc-gen-qml-core PRIVATE PBQML_COMPILER_HAS_QT)
  target_compile_definitions(protoc-gen-qml PRIVATE PBQML_COMPILER_HAS_QT)
endif()

target_link_libraries(protoc-gen-qml
  protoc-gen-qml-core
  protobuf-qml
  ${PROTOBUF_PROTOC_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  Qt5::Core)
if(BUILD_GRPC)
  target_link_libraries(protoc-gen-qml ${GRPC++_LIBRARIES})
endif()

#
# protobuf compiler sample (to run with valgrind)
#
add_executable(test-protoc-gen-qml
  protobuf/qml/qml_generator_test.cpp)
protobuf_qml_target(test-protoc-gen-qml)
target_link_libraries(test-protoc-gen-qml
  protoc-gen-qml-core)

