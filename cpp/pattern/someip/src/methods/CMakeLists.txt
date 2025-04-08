cmake_minimum_required(VERSION 2.8)
set(PRJ_NAME testingSIP)

set (COMMONAPI_DIR "/data001/tai2.tran/someip_stack/capicxx-core-runtime")
set (COMMONAPI_SOMEIP_DIR "/data001/tai2.tran/someip_stack/capicxx-someip-runtime")
set (VSOMEIP_DIR "/data001/tai2.tran/someip_stack/vsomeip")
set (ASYNC_TIGER_DIR "/data001/tai2.tran/codefun/cpp/pattern/dispatcher_v2")

set (LOCAL_LIB_DIR "/usr/local/lib")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++0x -Wl,--as-needed -std=c++17")


include_directories(
    src-gen
	src/include
	${COMMONAPI_DIR}/include
	${COMMONAPI_SOMEIP_DIR}/include
	${VSOMEIP_DIR}/interface
	${ASYNC_TIGER_DIR}/lib/include
)

link_directories(
	${COMMONAPI_DIR}/build
	${COMMONAPI_SOMEIP_DIR}/build
	${VSOMEIP_DIR}/build
	${ASYNC_TIGER_DIR}/build/lib
	${LOCAL_LIB_DIR}
)

set(LINK_LIBS libboost_system.so libboost_thread.so libprotobuf.so libasync_tiger.so)


# Source Files
set(PRJ_SRC_PATH src)
set(PRJ_SRC_GEN_PATH src-gen)
set(PRJ_SRC_GEN_COMMONAPI_PATH ${PRJ_SRC_GEN_PATH}/v1/taikt)

set(PRJ_NAME_CLIENT myClient)
set(PRJ_NAME_SERVICE myService)

# Application
FILE(GLOB PRJ_PROXY_GEN_SRCS ${PRJ_SRC_GEN_COMMONAPI_PATH}/*Proxy.cpp ${PRJ_SRC_GEN_COMMONAPI_PATH}/*Deployment.cpp)
FILE(GLOB PRJ_STUB_GEN_SRCS ${PRJ_SRC_GEN_COMMONAPI_PATH}/*Stub*.cpp ${PRJ_SRC_GEN_COMMONAPI_PATH}/*Deployment.cpp)
FILE(GLOB PRJ_STUB_IMPL_SRCS ${PRJ_SRC_COMMONAPI_PATH}/*Stub*.cpp)

set(PRJ_CLIENT_SRCS ${PRJ_SRC_PATH}/cli/Client.cpp ${PRJ_PROXY_GEN_SRCS})
set(PRJ_SERVICE_SRCS ${PRJ_SRC_PATH}/srv/Service.cpp ${PRJ_SRC_PATH}/srv/RequestManager.cpp ${PRJ_SRC_PATH}/srv/MsgHandler.cpp ${PRJ_SRC_PATH}/srv/SipStubImpl.cpp ${PRJ_STUB_GEN_SRCS} ${PRJ_STUB_IMPL_SRCS})

add_executable(${PRJ_NAME_CLIENT} ${PRJ_CLIENT_SRCS})
target_link_libraries(${PRJ_NAME_CLIENT} CommonAPI CommonAPI-SomeIP vsomeip3 ${LINK_LIBS})


add_executable(${PRJ_NAME_SERVICE} ${PRJ_SERVICE_SRCS})
target_link_libraries(${PRJ_NAME_SERVICE} CommonAPI CommonAPI-SomeIP vsomeip3 ${LINK_LIBS})


