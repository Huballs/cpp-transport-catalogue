cmake_minimum_required(VERSION 3.10)
project(bus-manager)

add_definitions("-std=c++17")

set(CMAKE_COLOR_DIAGNOSTICS ON)

set(CMAKE_PREFIX_PATH "/home/john/vscode/Protobuf/")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport-catalogue/transport_catalogue.proto)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(BUS_MANAGER_FILES transport-catalogue/main.cpp
                        transport-catalogue/input_reader.cpp
                        transport-catalogue/stat_reader.cpp
                        transport-catalogue/transport_catalogue.cpp
                        transport-catalogue/json.cpp
                        transport-catalogue/svg.cpp
                        transport-catalogue/json_reader.cpp
                        transport-catalogue/request_handler.cpp
                        transport-catalogue/map_renderer.cpp
                        transport-catalogue/json_builder.cpp
                        transport-catalogue/transport_router.cpp
                        transport-catalogue/seriallization.cpp
)

add_executable(bus-manager  ${PROTO_SRCS} 
                            ${PROTO_HDRS} 
                            ${BUS_MANAGER_FILES})

target_include_directories(bus-manager  PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(bus-manager  PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(bus-manager "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

