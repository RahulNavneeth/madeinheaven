cmake_minimum_required(VERSION 3.29)

project(unnamed)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VENDOR ${CMAKE_SOURCE_DIR}/vendor)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${VENDOR}/raylib/src)

add_subdirectory(${VENDOR}/raylib)

# file(GLOB SOURCES
# 	${CMAKE_SOURCE_DIR}/src/*.cpp
# )

add_executable(main main.cpp ${SOURCES})
target_link_libraries(main raylib)
