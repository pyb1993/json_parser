cmake_minimum_required (VERSION 3.1)
project (leptjson_test)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

set(CMAKE_CXX_STANDARD 11)
add_library(leptjson leptjson.cpp)
add_executable(leptjson_test test.cpp)
target_link_libraries(leptjson_test leptjson)
