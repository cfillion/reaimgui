find_package(GTest REQUIRED)
add_executable(tests
  color_test.cpp
  texture_test.cpp
)
target_link_libraries(tests PRIVATE GTest::gmock_main src)

include(GoogleTest)
gtest_discover_tests(tests)
