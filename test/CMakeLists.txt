if(BUILD_TESTING)
  add_subdirectory(protobuf)
endif()

if(BUILD_TESTING AND BUILD_GRPC OR BUILD_GRPC_TEST_SERVER)
  add_subdirectory(grpc)
endif()
