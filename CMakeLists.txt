cmake_minimum_required(VERSION 2.8.12)
project(protobuf-qml)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-inconsistent-missing-override HAS_W_INCOSISTENT_MISSING_OVERRIDE)
if(HAS_W_INCOSISTENT_MISSING_OVERRIDE)
  # Generated code for Q_OBJECT fires too many of this
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Protobuf REQUIRED)
find_package(Qt5Core COMPONENTS Private REQUIRED)
find_package(Qt5Qml COMPONENTS Private REQUIRED)
find_package(Qt5Quick COMPONENTS Private REQUIRED)

include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})
include_directories(compiler)

include_directories(SYSTEM
  ${Qt5Qml_PRIVATE_INCLUDE_DIRS})

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:QT_QML_DEBUG>)

find_package(GRPC++)
include(CMakeDependentOption)
cmake_dependent_option(BUILD_GRPC "Build gRPC QML module" ON
  "GRPC++_FOUND" OFF)
if(BUILD_GRPC)
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
      --plugin=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/protoc-gen-qml
      -I ${FIL_DIR}
      --qml_out  ${QML_OUT} ${ABS_FIL})
  endforeach()
endfunction()

if(CMAKE_VERSION VERSION_LESS 3.1.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")

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

if(BUILD_GRPC)
  add_subdirectory(lib/grpc)
endif()

#
# protobuf compiler plugin for QML/JS
#
set(protoc-gen-qml-core_SOURCES
  compiler/protobuf/qml/util.cpp
  compiler/protobuf/qml/enum_generator.cpp
  compiler/protobuf/qml/field_generator.cpp
  compiler/protobuf/qml/message_generator.cpp
  compiler/protobuf/qml/oneof_generator.cpp
  compiler/protobuf/qml/service_generator.cpp
  compiler/protobuf/qml/qml_generator.cpp
  compiler/protobuf/qml/compiler_util.cpp)
add_library(protoc-gen-qml-core
  ${protoc-gen-qml-core_SOURCES})
target_compile_definitions(protoc-gen-qml-core PRIVATE PBQML_COMPILER_HAS_QT)
target_link_libraries(protoc-gen-qml-core
  protobuf-qml
  ${PROTOBUF_PROTOC_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  Qt5::Core)

set(protoc-gen-qml_SOURCES
  compiler/protobuf/qml/main.cpp)
add_executable(protoc-gen-qml ${protoc-gen-qml_SOURCES})
protobuf_qml_target(protoc-gen-qml)
option(PROTOBUF_COMPILER_HAS_QT "Allow compiler plugin dependency on Qt" ON)
if(PROTOBUF_COMPILER_HAS_QT)
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
  compiler/protobuf/qml/qml_generator_test.cpp)
protobuf_qml_target(test-protoc-gen-qml)
target_link_libraries(test-protoc-gen-qml
  protoc-gen-qml-core)

include(CTest)
if(BUILD_TESTING)
  find_package(Qt5QuickTest REQUIRED)
  enable_testing()
  add_subdirectory(test)
endif()
