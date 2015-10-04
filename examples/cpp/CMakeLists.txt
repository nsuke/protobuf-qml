protobuf_generate_cpp(CPP_EXAMPLE_GEN CPP_EXAMPLE_GEN_H
  ${CMAKE_CURRENT_SOURCE_DIR}/cpp_example.proto)

include_directories(${PROTOBUF_INCLUDE_DIRS})
add_executable(CppExample main.cpp ${CPP_EXAMPLE_GEN})
target_link_libraries(CppExample Qt5::Qml Qt5::Gui ${PROTOBUF_LIBRARIES} protobuf-qml-core)
PROTOBUF_QML_TARGET(CppExample)


protobuf_generate_qml(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/cpp_example.proto)

copy(examples ${CMAKE_CURRENT_BINARY_DIR}
  main.qml)
