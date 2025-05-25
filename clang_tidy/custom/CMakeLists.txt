set(LLVM_LINK_COMPONENTS support)
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp)
add_clang_library(clangTidyCustomModule
  CustomTidyModule.cpp
  SingleReturnCheck.cpp
)
# Thêm các thư mục include bằng cách sử dụng biến CMake
target_include_directories(clangTidyCustomModule PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/../clang-tools-extra/clang-tidy
  ${CMAKE_SOURCE_DIR}/../clang-tools-extra/
  ${CMAKE_SOURCE_DIR}/../clang/include
  ${CMAKE_BINARY_DIR}/include
)

message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

# Thiết lập thuộc tính cho target
set_target_properties(clangTidyCustomModule PROPERTIES
  LINKER_LANGUAGE CXX
  CXX_STANDARD 17
)

# Liên kết với các thư viện Clang cần thiết
target_link_libraries(clangTidyCustomModule
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTidy
  clangTidyUtils
)