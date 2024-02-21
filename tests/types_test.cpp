#include "../api/types.hpp"

#include <gtest/gtest.h>

namespace std {
  template<size_t N, typename T>
  bool operator==(const std::array<char, N> &array, const T value)
  {
    return std::string_view { array.data(), array.size() } == value;
  }
}

TEST(TypesTest, Builtins) {
  EXPECT_EQ(TypeInfo<void       >::type(), "void");
  EXPECT_EQ(TypeInfo<void*      >::type(), "void*");
  EXPECT_EQ(TypeInfo<bool       >::type(), "bool");
  EXPECT_EQ(TypeInfo<bool*      >::type(), "bool*");
  EXPECT_EQ(TypeInfo<int        >::type(), "int");
  EXPECT_EQ(TypeInfo<int*       >::type(), "int*");
  EXPECT_EQ(TypeInfo<double     >::type(), "double");
  EXPECT_EQ(TypeInfo<double*    >::type(), "double*");
  EXPECT_EQ(TypeInfo<char*      >::type(), "char*");
  EXPECT_EQ(TypeInfo<const char*>::type(), "const char*");
}

struct MyObjectA; struct MyObjectB;
API_REGISTER_OBJECT_TYPE(MyObjectA);
API_REGISTER_TYPE(MyObjectB*, "ChunkyBacon*");

TEST(TypesTest, Objects) {
  EXPECT_EQ(TypeInfo<MyObjectA   *>::type(), "ImGui_MyObjectA*");
  EXPECT_EQ(TypeInfo<MyObjectB   *>::type(), "ChunkyBacon*");
  EXPECT_EQ(TypeInfo<reaper_array*>::type(), "reaper_array*");
}
