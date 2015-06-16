cmake_minimum_required(VERSION 2.8.12)
project(protobuf-qml)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # TODO: check clang version
  # Generated code for Q_OBJECT fires too many of this
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
endif()

find_package(Protobuf REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)

include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})
include_directories(compiler)

find_package(GRPC++)
if(GRPC++_FOUND)
  include_directories(SYSTEM ${GRPC++_INCLUDE_DIRS})
endif()

function(copy TAG DEST)
  foreach(FIL ${ARGN})
    get_filename_component(FIL_ABS ${FIL} ABSOLUTE)
    get_filename_component(FIL_NAME ${FIL} NAME)
    set(DST ${DEST}/${FIL_NAME})
    add_custom_command(OUTPUT ${DST} DEPENDS ${FIL_ABS}
      COMMAND ${CMAKE_COMMAND} -E copy ${FIL_ABS} ${DST})
    add_custom_target("copy-${TAG}-${FIL_NAME}" ALL DEPENDS ${DST})
  endforeach()
endfunction()

function(protobuf_generate_qml QML_OUT)
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    get_filename_component(FIL_DIR ${FIL} DIRECTORY)

    add_custom_target("protobuf_generate_qml_${FIL_WE}" ALL
      DEPENDS ${ABS_FIL} protoc-gen-qml
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      --plugin=${CMAKE_BINARY_DIR}/protoc-gen-qml
      -I ${FIL_DIR}
      --qml_out  ${QML_OUT} ${ABS_FIL})
  endforeach()
endfunction()

if(CMAKE_VERSION VERSION_LESS 3.1.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

function(PROTOBUF_QML_TARGET TARGET)
  if(NOT CMAKE_VERSION VERSION_LESS 3.1.0)
    set(CXX_FEATURES
      cxx_auto_type
      cxx_lambdas
      cxx_nullptr
      cxx_override
      cxx_range_for)
    target_compile_features(${TARGET} PUBLIC ${CXX_FEATURES})
  endif()
endfunction()

set(PROTOBUF_QML_TARGETS protobuf-qml protoc-gen-qml)

include_directories(lib)
add_subdirectory(lib/protobuf)

if(GRPC++_FOUND)
  add_subdirectory(lib/grpc)
endif()

set(protoc-gen-qml_SOURCES
  compiler/protobuf/qml/main.cpp
  compiler/protobuf/qml/util.cpp
  compiler/protobuf/qml/enum_generator.cpp
  compiler/protobuf/qml/field_generator.cpp
  compiler/protobuf/qml/message_generator.cpp
  compiler/protobuf/qml/oneof_generator.cpp
  compiler/protobuf/qml/qml_generator.cpp
  compiler/protobuf/qml/compiler_util.cpp)
add_executable(protoc-gen-qml ${protoc-gen-qml_SOURCES})
protobuf_qml_target(protoc-gen-qml)
target_compile_definitions(protoc-gen-qml PRIVATE PBQML_COMPILER_HAS_QT)
target_link_libraries(protoc-gen-qml
  protobuf-qml
  ${PROTOBUF_PROTOC_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  Qt5::Core)
if(GRPC++_FOUND)
  target_link_libraries(protoc-gen-qml ${GRPC++_LIBRARIES})
endif()

copy(protobuf-qml-test
  ${CMAKE_CURRENT_BINARY_DIR}/test
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_jsobject.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_message.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_nesting.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_oneof.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_parse_error.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_primitive.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_repeated.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_serialize_error.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/test/tst_file.qml
  )

find_package(Qt5QuickTest REQUIRED)
add_executable(protobuf-qml-test test/test_main.cpp)
protobuf_qml_target(protobuf-qml-test)
target_link_libraries(protobuf-qml-test Qt5::QuickTest)
enable_testing()
add_test(NAME protobuf-qml-test
  COMMAND ./protobuf-qml-test
  -import ${CMAKE_CURRENT_BINARY_DIR}/plugins
  -input ${CMAKE_CURRENT_BINARY_DIR}/test)

protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}/test
  ${CMAKE_CURRENT_SOURCE_DIR}/test/ProtobufQmlTest.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/test/ProtobufQmlTest2.proto)
