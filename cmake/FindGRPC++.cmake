# in: GRPC_ROOT
# out: GRPC++_FOUND GRPC++_LIBRARIES GRPC++_INCLUDE_DIRS

message(STATUS "ROOT : " ${GRPC_ROOT})

find_package(Protobuf CONFIG)

macro(grpc_generate_cxx GEN)
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    get_filename_component(FIL_DIR ${FIL} DIRECTORY)

    set(${GEN}
      "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.cc"
      "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.h"
      "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
      "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
      )

  message(STATUS "GEN:: ${GEN}")
  message(STATUS "GEN:: ${${GEN}}")

    add_custom_command(
      OUTPUT ${${GEN}}
      COMMAND protobuf::protoc
      --plugin=protoc-gen-grpc=${GRPCXX_PLUGIN_EXECUTABLE}
      --cpp_out ${CMAKE_CURRENT_BINARY_DIR}
      --grpc_out ${CMAKE_CURRENT_BINARY_DIR}
      -I ${CMAKE_CURRENT_SOURCE_DIR}
      ${ABS_FIL}
      DEPENDS ${ABS_FIL} protobuf::protoc ${GRPCXX_PLUGIN_EXECUTABLE}
      COMMENT "Running C++ protocol buffer compiler with gRPC++ plugin on ${FIL}"
      VERBATIM)
  endforeach()
endmacro()

find_path(GRPC_INCLUDE_DIR
  NAMES grpc/grpc.h
  PATHS ${GRPC_ROOT}/include)
mark_as_advanced(GRPC_INCLUDE_DIR)

find_library(GPR_LIBRARY
  NAMES gpr
  PATHS ${GRPC_ROOT}/lib)
mark_as_advanced(GPR_LIBRARY)

find_library(GRPC_LIBRARY
  NAMES grpc
  PATHS ${GRPC_ROOT}/lib)
mark_as_advanced(GRPC_LIBRARY)

find_library(GRPCXX_LIBRARY
  NAMES grpc++
  PATHS ${GRPC_ROOT}/lib)
mark_as_advanced(GRPCXX_LIBRARY)

find_library(SSL_LIBRARY
  NAMES libssl.so ssl
  NO_DEFAULT_PATH
  PATHS ${GRPC_ROOT}/lib)
mark_as_advanced(SSL_LIBRARY)
message(STATUS "SSL : ${SSL_LIBRARY}")

find_library(CRYPTO_LIBRARY
  NAMES libcrypto.so crypto
  NO_DEFAULT_PATH
  PATHS ${GRPC_ROOT}/lib)
mark_as_advanced(CRYPTO_LIBRARY)
message(STATUS "CRYPTO : ${CRYPTO_LIBRARY}")

  find_library(Z_LIBRARY
    NAMES z zlib
    PATHS ${GRPC_ROOT}/lib)
  mark_as_advanced(Z_LIBRARY)

find_program(GRPCXX_PLUGIN_EXECUTABLE
  NAMES grpc_cpp_plugin
  PATHS ${GRPC_ROOT}/bin)
mark_as_advanced(GRPCXX_PLUGIN_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GRPC++ DEFAULT_MSG
  GRPC_INCLUDE_DIR GPR_LIBRARY GRPC_LIBRARY GRPCXX_LIBRARY GRPCXX_PLUGIN_EXECUTABLE)

if(GRPC++_FOUND)
  set(GRPC++_INCLUDE_DIRS ${GRPC_INCLUDE_DIR})
  set(GRPC++_INCLUDE_DIRS ${GRPC_INCLUDE_DIR})
  if(MSVC)
    set(GRPC++_LIBRARIES ${GPR_LIBRARY} ${GRPC_LIBRARY} ${GRPCXX_LIBRARY} ${SSL_LIBRARY} ${CRYPTO_LIBRARY} ${Z_LIBRARY} ws2_32 protobuf::libprotobuf)
  else()
    set(GRPC++_LIBRARIES ${GPR_LIBRARY} ${GRPC_LIBRARY} ${GRPCXX_LIBRARY} ${SSL_LIBRARY} ${CRYPTO_LIBRARY} ${Z_LIBRARY} protobuf::libprotobuf)
  endif()
endif()
