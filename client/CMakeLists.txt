cmake_minimum_required(VERSION 3.22)

project(client)

set(CMAKE_CXX_STANDARD 17)

# Abseil dependencies
find_package(abseil)

# Protobuf dependencies
set(protobuf_MODULE_COMPATIBLE TRUE)

find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

# Protobuf variables
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

get_filename_component(numbers_proto "../proto/numbers.proto" ABSOLUTE)
get_filename_component(numbers_proto_path "${numbers_proto}" PATH)

set(numbers_proto_sources "${CMAKE_CURRENT_BINARY_DIR}/numbers.pb.cc")
set(numbers_proto_headers "${CMAKE_CURRENT_BINARY_DIR}/numbers.pb.h")
set(numbers_grpc_sources "${CMAKE_CURRENT_BINARY_DIR}/numbers.grpc.pb.cc")
set(numbers_grpc_headers "${CMAKE_CURRENT_BINARY_DIR}/numbers.grpc.pb.h")

add_custom_command(
        OUTPUT "${numbers_proto_sources}" "${numbers_proto_headers}" "${numbers_grpc_sources}" "${numbers_grpc_headers}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${numbers_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${numbers_proto}"
        DEPENDS "${numbers_proto}"
)

# Protobuf and binary generation
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(numbers_grpc_proto
        ${numbers_proto_sources}
        ${numbers_proto_headers}
        ${numbers_grpc_sources}
        ${numbers_grpc_headers}
)

target_link_libraries(numbers_grpc_proto
        ${_PROTOBUF_LIBPROTOBUF}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
)

foreach(_target client client_async)
    add_executable(${_target} "${_target}.cc")

    target_link_libraries(${_target}
            numbers_grpc_proto
            absl::flags
            absl::flags_parse
            ${_PROTOBUF_LIBPROTOBUF}
            ${_REFLECTION}
            ${_GRPC_GRPCPP}
    )
endforeach()