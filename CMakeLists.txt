cmake_minimum_required(VERSION 3.1.0)
project(protobuf-qml VERSION 0.0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Protobuf REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(compiler)
include_directories(lib)

add_executable(protoc-gen-qml
  compiler/protobuf/qml/main.cpp
  compiler/protobuf/qml/enum_generator.cpp
  compiler/protobuf/qml/message_generator.cpp
  compiler/protobuf/qml/field_generator.cpp
  compiler/protobuf/qml/qml_generator.cpp
  )

target_link_libraries(protoc-gen-qml
  ${PROTOBUF_PROTOC_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  Qt5::Core
  )

add_library(protobuf-qml SHARED
  lib/protobuf/qml/descriptor_database.cpp
  lib/protobuf/qml/file.cpp
  lib/protobuf/qml/io.cpp
  lib/protobuf/qml/memory.cpp
  lib/protobuf/qml/plugin.cpp
  lib/protobuf/qml/wire_format_lite.cpp)

target_compile_definitions(protobuf-qml PRIVATE -DPROTOBUF_QML_EXPORT)
set(CXX_FEATURES
  cxx_auto_type
  cxx_lambdas
  cxx_nullptr
  cxx_override
  cxx_range_for)

target_compile_features(protobuf-qml PUBLIC ${CXX_FEATURES})
target_compile_features(protoc-gen-qml PUBLIC ${CXX_FEATURES})

target_link_libraries(
  protobuf-qml
  ${PROTOBUF_LIBRARIES}
  Qt5::Qml)

function (copy DEST)
  foreach(FIL ${ARGN})
    get_filename_component(FIL_ABS ${FIL} ABSOLUTE)
    get_filename_component(FIL_NAME ${FIL} NAME)
    set(DST ${DEST}/${FIL_NAME})
    add_custom_command(OUTPUT ${DST} DEPENDS ${FIL_ABS}
      COMMAND ${CMAKE_COMMAND} -E copy ${FIL_ABS} ${DST})
    add_custom_target("copy-qml-${FIL_NAME}" ALL DEPENDS ${DST})
  endforeach()
endfunction()

copy(${CMAKE_CURRENT_BINARY_DIR}/plugins/Protobuf
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/qml/qmldir
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/qml/Async.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/qml/message.js
  ${CMAKE_CURRENT_BINARY_DIR}/libprotobuf-qml.so)

copy(${CMAKE_CURRENT_BINARY_DIR}/test ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_protobuf_qml.qml)
find_package(Qt5QuickTest REQUIRED)
add_executable(protobuf-qml-test test/test_main.cpp)
target_link_libraries(protobuf-qml-test Qt5::QuickTest)
enable_testing()
add_test(NAME protobuf-qml-test
  COMMAND ./protobuf-qml-test
  -import ${CMAKE_CURRENT_BINARY_DIR}/plugins
  -input ${CMAKE_CURRENT_BINARY_DIR}/test
  )

function (protobuf_generate_qml QML_OUT)
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    get_filename_component(FIL_DIR ${FIL} DIRECTORY)

    add_custom_target("protobuf_generate_qml_${FIL_WE}" ALL
      DEPENDS ${ABS_FIL} protoc-gen-qml
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      --plugin=protoc-gen-qml
      -I ${FIL_DIR}
      --qml_out  ${QML_OUT} ${ABS_FIL})
  endforeach()
endfunction()

protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}/test
  ${CMAKE_CURRENT_SOURCE_DIR}/test/ProtobufQmlTest.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/test/ProtobufQmlTest2.proto)
