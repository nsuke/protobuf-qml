find_package(Qt5Test COMPONENTS Private REQUIRED)

protobuf_generate_cxx(test_convert_GEN test_convert.proto)

add_executable(TestConvert
  test_convert.cpp
  ${test_convert_GEN}
  )

target_link_libraries(TestConvert
  Qt5::Quick
  Qt5::Test
  protobuf-qml-core
  ${PROTOBUF_LIBRARIES})


protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_convert.proto)

copy(TestConvert
  ${CMAKE_CURRENT_BINARY_DIR}
  test_convert.qml)

add_test(NAME TestConvert
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TestConvert)
