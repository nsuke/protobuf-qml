add_executable(XMLHttpRequestSample client/xhr_client.cpp)
target_link_libraries(XMLHttpRequestSample Qt5::Qml Qt5::Gui)

protobuf_generate_lang(python ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/my_message.proto)

protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/my_message.proto)

copy(examples ${CMAKE_CURRENT_BINARY_DIR}
  server/server.py
  client/XhrClient.qml)
