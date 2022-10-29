find_package(Catch2 3 REQUIRED)
add_executable(tests
  color_test.cpp
  texture_test.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain src)

include(Catch)
catch_discover_tests(tests)
