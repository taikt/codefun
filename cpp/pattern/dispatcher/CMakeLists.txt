cmake_minimum_required(VERSION 3.0)
project(binary_build)

#include_directories(include /Users/taikt/Documents/code/codefun/cpp/cxx/cmake/sharedLib/include)
include_directories(include)
set(CMAKE_CXX_FLAGS "-std=c++14 -pthread")

#set(LINK_LIBS libqueuelib.dylib)
#link_directories(/Users/taikt/Documents/code/codefun/cpp/cxx/cmake/sharedLib/build)

file(GLOB SOURCES "src/*.cpp")

add_executable(a.out ${SOURCES})

#target_link_libraries(foo.out ${LINK_LIBS})

