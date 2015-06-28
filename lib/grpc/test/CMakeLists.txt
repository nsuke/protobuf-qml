#
# QML client test
#
copy(grpc-qml-test
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/tst_grpc_channel.qml
  )

protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/hello.proto)

#
# C++ test server
#
grpc_generate_cxx(hello.proto)

set(hello-server_SOURCES
  server.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/hello.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/hello.grpc.pb.cc)

add_executable(hello-server ${hello-server_SOURCES})

target_link_libraries(hello-server ${GRPC++_LIBRARIES} ${PROTOBUF_LIBRARIES})

protobuf_qml_target(hello-server)

#
# C++ test async server
#
grpc_generate_cxx(hello.proto)

set(hello-async-server_SOURCES
  async_server.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/hello.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/hello.grpc.pb.cc)

add_executable(hello-async-server ${hello-async-server_SOURCES})

target_link_libraries(hello-async-server ${GRPC++_LIBRARIES} ${PROTOBUF_LIBRARIES})

protobuf_qml_target(hello-async-server)
