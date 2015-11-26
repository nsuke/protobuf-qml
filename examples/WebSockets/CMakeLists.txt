add_executable(WebSocketsSampleClient client/client.cpp)
target_link_libraries(WebSocketsSampleClient Qt5::Qml Qt5::Gui)

add_executable(WebSocketsSampleServer server/server.cpp)
target_link_libraries(WebSocketsSampleServer Qt5::Qml Qt5::Gui)

protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/foo.proto)

copy(examples ${CMAKE_CURRENT_BINARY_DIR}
  server/server.qml
  client/client.qml)

