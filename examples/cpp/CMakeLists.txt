protobuf_generate_cxx(cpp_example_GEN cpp_example.proto)

add_executable(CppExample main.cpp ${cpp_example_GEN})
target_link_libraries(CppExample Qt5::Qml Qt5::Gui protobuf-qml-core)
PROTOBUF_QML_TARGET(CppExample)


protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/cpp_example.proto)

copy(examples ${CMAKE_CURRENT_BINARY_DIR}
  main.qml)
