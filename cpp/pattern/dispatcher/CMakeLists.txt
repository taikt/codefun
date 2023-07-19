cmake_minimum_required(VERSION 3.0)
project(binary_build)

#include_directories(include /Users/taikt/Documents/code/codefun/cpp/cxx/cmake/sharedLib/include)
include_directories(include)
set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")

set(LINK_LIBS libboost_system.so libboost_thread.so libprotobuf.so)
link_directories(/usr/local/lib)

file(GLOB SOURCES "src/*.cpp" "src/*.cc")

add_executable(a.out ${SOURCES})

target_link_libraries(a.out ${LINK_LIBS})

