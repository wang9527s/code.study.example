cmake_minimum_required(VERSION 3.8)

project(HelloWorld C CXX)

find_package(Threads REQUIRED)
# 获取 Git 根目录
execute_process(
    COMMAND git rev-parse --show-toplevel
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_ROOT_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 输出 Git 根目录
message(STATUS "Git root directory: ${GIT_ROOT_DIR}")
set(GRPC_INSTALL_DIR "${GIT_ROOT_DIR}/lib/grpc-1.65.3")

set(absl_DIR ${GRPC_INSTALL_DIR}/lib/cmake/absl)
set(utf8_range_DIR ${GRPC_INSTALL_DIR}/lib/cmake/utf8_range)

# 设置 Protobuf 查找路径
find_package(Protobuf CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/protobuf NO_DEFAULT_PATH)
message(STATUS "Protobuf_DIR package found at: ${Protobuf_DIR} ${Protobuf_VERSION}")
find_package(gRPC CONFIG REQUIRED PATHS ${GRPC_INSTALL_DIR}/lib/cmake/grpc NO_DEFAULT_PATH)
message(STATUS "gRPC package found at: ${gRPC_DIR} ${gRPC_VERSION}")

# 生成pb文件
get_filename_component(hw_proto "helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.h")

# /home/wangbin/grpc/install/bin/protoc --grpc_out=. --cpp_out=. -I /home/wangbin/grpc/helloworld 
#           --plugin=protoc-gen-grpc=/home/wangbin/grpc/install/bin/grpc_cpp_plugin helloworld.proto
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${GRPC_INSTALL_DIR}/bin/protoc
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${GRPC_INSTALL_DIR}/bin/grpc_cpp_plugin"
        "${hw_proto}"
      DEPENDS "${hw_proto}"
)

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

foreach(_target
  greeter_client greeter_server)
  add_executable(${_target} "${_target}.cc"
    ${hw_grpc_srcs}
    ${hw_grpc_hdrs}
    ${hw_proto_srcs}
    ${hw_proto_hdrs}
  )
  target_link_libraries(${_target}
    gRPC::grpc++
    gRPC::grpc++_reflection
    ${GRPC_INSTALL_DIR}/lib/libprotobuf.a
  )
endforeach()
