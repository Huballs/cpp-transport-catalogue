cmake_minimum_required(VERSION 2.8.12)
project(bus-manager)

add_definitions("-std=c++17")

set(CMAKE_COLOR_DIAGNOSTICS ON)
#set(DCMAKE_PREFIX_PATH "C:\\msys64\\ucrt64\\include\\tbb")

#find_package(TBB REQUIRED)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow")

if (UNIX)
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltbb -lpthread") 
endif (UNIX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


add_executable(bus-manager transport-catalogue/main.cpp
    transport-catalogue/input_reader.cpp
    transport-catalogue/stat_reader.cpp
    transport-catalogue/transport_catalogue.cpp
    transport-catalogue/json.cpp
)

#target_link_libraries(search-server PRIVATE TBB::tbb)

add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
    
        COMMAND ${CMAKE_OBJDUMP} -h -S $<TARGET_FILE:${PROJECT_NAME}> > ${PROJECT_NAME}.lst)
