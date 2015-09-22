#
# protobuf-qml QML tests
#
copy(protobuf-qml-test
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_map.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_serialization.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_benchmark.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_binary.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_jsobject.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_message.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_nesting.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_recursive.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_oneof.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_parse_error.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_primitive.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_repeated.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_serialize_error.qml
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_file.qml
  )

protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/maps.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/serialization_test.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/recursive_test.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/oneof_test.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/ProtobufQmlTest.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/ProtobufQmlTest2.proto)


#
# QML test runner
#
add_executable(test-protobuf-qml test_main.cpp)
protobuf_qml_target(test-protobuf-qml)
target_compile_definitions(test-protobuf-qml PRIVATE
  QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(test-protobuf-qml Qt5::QuickTest)
add_test(NAME test-protobuf-qml COMMAND
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-protobuf-qml)
