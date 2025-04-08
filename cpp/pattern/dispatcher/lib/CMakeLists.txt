# http://derekmolloy.ie/hello-world-introductions-to-cmake/

cmake_minimum_required(VERSION 3.0)
#project(dispatcher_lib)

include_directories(include)
set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")

set(LINK_LIBS libboost_system.so libboost_thread.so libprotobuf.so)
link_directories(/usr/local/lib)

file(GLOB SOURCES "src/*.cpp")

add_library(async_tiger SHARED ${SOURCES})

target_link_libraries(async_tiger ${LINK_LIBS})
target_include_directories(async_tiger PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

# sudo make install
install(TARGETS async_tiger DESTINATION /usr/local/lib)