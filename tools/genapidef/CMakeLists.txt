add_executable(genapidef EXCLUDE_FROM_ALL genapidef.cpp)
target_link_libraries(genapidef common)

find_package(Clang REQUIRED CONFIG)
target_include_directories(genapidef PRIVATE
  ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
target_compile_definitions(genapidef PRIVATE
  ${LLVM_DEFINITIONS} ${CLANG_DEFINITIONS})
target_link_libraries(genapidef clangFrontend clangTooling)
