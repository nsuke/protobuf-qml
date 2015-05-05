
set(grpc-qml_SOURCES
  qml/channel.cpp
  qml/credentials.cpp
  qml/grpc_plugin.cpp
  qml/global_instance.cpp
  )
add_library(grpc-qml SHARED ${grpc-qml_SOURCES})
protobuf_qml_target(grpc-qml)
target_link_libraries(grpc-qml
  ${GRPC++_LIBRARIES}
  Qt5::Qml)

copy(grpc-qml
  ${CMAKE_BINARY_DIR}/plugins/Grpc
  imports/qmldir
  ${CMAKE_CURRENT_BINARY_DIR}/libgrpc-qml.so)
