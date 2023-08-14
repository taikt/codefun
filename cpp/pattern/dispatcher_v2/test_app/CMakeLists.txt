add_compile_options(
    -Wno-error
)

set (CMAKE_CXX_STANDARD 17)


set(LINK_LIBS libboost_system.so libboost_thread.so libprotobuf.so libasync_tiger.so)

link_directories(/usr/lib)

#file(GLOB SOURCES "src/*.cpp" "src/*.cc")
file(GLOB SOURCES "src/main_test.cpp")

add_executable(asynctest_app ${SOURCES})

target_link_libraries(asynctest_app ${LINK_LIBS})

# https://stackoverflow.com/questions/48428647/how-to-use-cmake-to-install
# install(TARGETS asynctest_app DESTINATION /usr/bin)

install(TARGETS asynctest_app DESTINATION /usr/bin)




