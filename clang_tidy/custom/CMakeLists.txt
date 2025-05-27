set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyCustomModule
  CustomTidyModule.cpp
  SingleReturnCheck.cpp
  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangTidy
  clangTidyUtils
  
  DEPENDS
  omp_gen
  ClangDriverOptions
)

target_include_directories(clangTidyCustomModule
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../clang/include
)

clang_target_link_libraries(clangTidyCustomModule
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTooling
  )