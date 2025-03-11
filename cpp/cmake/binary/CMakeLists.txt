# http://derekmolloy.ie/hello-world-introductions-to-cmake/

cmake_minimum_required(VERSION 3.0)
project(binary_build)

include_directories(include /Users/taikt/Documents/code/codefun/cpp/cxx/cmake/sharedLib/include)

set(LINK_LIBS libqueuelib.dylib)
link_directories(/Users/taikt/Documents/code/codefun/cpp/cxx/cmake/sharedLib/build)

file(GLOB SOURCES "src/*.cpp")

add_executable(foo.out ${SOURCES})

target_link_libraries(foo.out ${LINK_LIBS})

#[[
cmake_minimum_required(VERSION 3.15)
project(example_server)
 
include_directories(./../include /usr/local/include/NeoFramework-5/Nf5Core/ /usr/local/include/MCP-0.0/ ./../../../common/util/src/include/)

set(CMAKE_CXX_FLAGS "-DBOOST_ALL_NO_LIB -DBOOST_PROGRAM_OPTIONS_DYN_LINK \
-DFMT_LOCALE -DFMT_SHARED -DMSC_PLATFORM_MGU18 -DNF_LOG_CONTEXT=demo_log \
-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,--as-needed -std=c++20")

set (PROJECT_LINK_LIBS libMcpClientCommon.so libMcpClientRaw.so libNf5Core.so libmcp-common-util.a libprotobuf.so libfmt.so libboost_program_options.so)
link_directories(/usr/local/lib /usr/lib/x86_64-linux-gnu/)
 
set(SOURCES src/ServerApplication.cpp src/main.cpp src/NeoApplication.cpp ./../common/BaseMCPApplication.cpp ./../common/helloWorld.pb.cc ./../common/options.pb.cc)
 
add_executable(app-server ${SOURCES})

target_link_libraries(app-server ${PROJECT_LINK_LIBS} )
]]

