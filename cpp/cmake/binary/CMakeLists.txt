# http://derekmolloy.ie/hello-world-introductions-to-cmake/

cmake_minimum_required(VERSION 2.8.9)
project(binary_build)

include_directories(include /Users/taikt/Documents/code/codefun/cpp/cxx/cmake/sharedLib/include)

set(LINK_LIBS libqueuelib.dylib)
link_directories(/Users/taikt/Documents/code/codefun/cpp/cxx/cmake/sharedLib/build)

file(GLOB SOURCES "src/*.cpp")

add_executable(foo.out ${SOURCES})

target_link_libraries(foo.out ${LINK_LIBS})

