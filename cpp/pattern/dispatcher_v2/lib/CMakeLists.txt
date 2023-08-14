#[[
    https://stackoverflow.com/questions/10851247/how-do-i-activate-c-11-in-cmake
    set(CMAKE_CXX_FLAGS "-std=c++17") doesn't work for some cmake VERSION

    use below instead 
    set (CMAKE_CXX_STANDARD 17)
    or 
    add_definitions(
    -std=c++17
    )
]]

add_compile_options(
    -Wno-error
)

include_directories(include)
set (CMAKE_CXX_STANDARD 17)

set(LINK_LIBS libboost_system.so libboost_thread.so libprotobuf.so)
#link_directories(/usr/local/lib)
link_directories(/usr/lib)

file(GLOB SOURCES "src/*.cpp")


#file(GLOB HEADER_FILES "Dispatcher.h;Continuation.h")
file(GLOB HEADER_FILES "include/*.h")

add_library(async_tiger SHARED ${SOURCES})

target_link_libraries(async_tiger ${LINK_LIBS})
target_include_directories(async_tiger PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

# sudo make install
#install(TARGETS async_tiger DESTINATION /usr/lib)
#set_target_properties(async_tiger PROPERTIES PUBLIC_HEADER "Dispatcher.h;Continuation.h")
# https://stackoverflow.com/questions/10487256/cmake-how-to-properly-copy-static-librarys-header-file-into-usr-include
#working below
set_target_properties(async_tiger PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
install(TARGETS async_tiger 
        LIBRARY DESTINATION /usr/lib
        PUBLIC_HEADER DESTINATION /usr/include/
)

#install(TARGETS async_tiger DESTINATION /usr/lib)





