cmake_minimum_required(VERSION 3.10)
project(libcoroutine VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -fcoroutines")

message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")

include_directories(src)

set(SRC src/state.cpp)

add_library(libcoroutine STATIC ${SRC})
set_property(TARGET libcoroutine  PROPERTY CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)