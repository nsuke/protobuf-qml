if(BUILD_TESTING)
  #
  # grpc-qml QML tests
  #
  copy(grpc-qml-test
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common.js
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_grpc_channel.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_grpc_server.qml
    )

  protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/hello.proto)

  #
  # QML test runner
  #
  add_executable(test-grpc-qml test_main.cpp)
  protobuf_qml_target(test-grpc-qml)
  target_compile_definitions(test-grpc-qml PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}")
  target_link_libraries(test-grpc-qml Qt5::QuickTest)
  add_test(NAME test-grpc-qml COMMAND
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test-grpc-qml
    -input ${CMAKE_CURRENT_BINARY_DIR}/tst_grpc_server.qml)
endif()

#
# C++ test server
#
grpc_generate_cxx(hello.proto)

set(test-hello-server_SOURCES
  server.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/hello.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/hello.grpc.pb.cc)

add_executable(test-hello-server ${test-hello-server_SOURCES})

target_link_libraries(test-hello-server ${GRPC++_LIBRARIES} ${PROTOBUF_LIBRARIES})
if(UNIX)
  target_link_libraries(test-hello-server rt)
endif()

protobuf_qml_target(test-hello-server)

#
# C++ test async server
#
set(test-hello-async-server_SOURCES
  async_server.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/hello.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/hello.grpc.pb.cc)

add_executable(test-hello-async-server ${test-hello-async-server_SOURCES})

target_link_libraries(test-hello-async-server ${GRPC++_LIBRARIES} ${PROTOBUF_LIBRARIES})
if(UNIX)
  target_link_libraries(test-hello-async-server rt)
endif()
protobuf_qml_target(test-hello-async-server)
