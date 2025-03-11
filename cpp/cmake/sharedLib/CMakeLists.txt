# http://derekmolloy.ie/hello-world-introductions-to-cmake/

cmake_minimum_required(VERSION 3.0)
project(shared_lib_test)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_library(queuelib SHARED ${SOURCES})

# sudo make install
install(TARGETS queuelib DESTINATION /usr/local/lib)