cmake_minimum_required(VERSION 3.10)
project(libcoroutine VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")


set(LIBCORO_DEBUG 1)

add_definitions(-DLIBCORO_DEBUG_PTR
        -D_DEBUG)

message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")

include_directories(libcoroutine)


#file(GLOB_RECURSE SRC LIST_DIRECTORIES true libcoroutine/src/*.h libcoroutine/src/*.cpp)
set(SRC libcoroutine/src/state.cpp
        libcoroutine/src/scheduler.cpp
        libcoroutine/src/timer.cpp
        libcoroutine/src/sleep.cpp)

add_library(libcoroutine STATIC ${SRC})
add_library(libcoroutine_SHARED SHARED ${SRC})


target_link_libraries(libcoroutine pthread -fcoroutines)

set_property(TARGET libcoroutine  PROPERTY CXX_STANDARD 20)
set_target_properties(libcoroutine PROPERTIES  OUTPUT_NAME "libcoroutine")
set_target_properties(libcoroutine_SHARED PROPERTIES OUTPUT_NAME "libcoroutine")

# cmake在构建一个新的target时，会尝试清理掉其他使用这个名字的库，
# 因此，libcoroutine.a时，libcoroutine.so.
# 为了回避这个问题，比如再次使用SET_TARGET_PROPERTIES定义 CLEAN_DIRECT_OUTPUT属性。
set_target_properties(libcoroutine PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(libcoroutine_SHARED PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# VERSION指代动态库版本，SOVERSION指代API版本
set_target_properties(libcoroutine_SHARED PROPERTIES VERSION 1.0 SOVERSION 1 )

set_target_properties(libcoroutine PROPERTIES PREFIX "")
set_target_properties(libcoroutine_SHARED PROPERTIES PREFIX "")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(test)