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

  EXPECT_EQ(TypeInfo<int>::name(), "_");
}

struct MyObjectA; struct MyObjectB;
API_REGISTER_OBJECT_TYPE(MyObjectA);
API_REGISTER_TYPE(MyObjectB*, "ChunkyBacon*");

TEST(TypesTest, Objects) {
  EXPECT_EQ(TypeInfo<MyObjectA   *>::type(), "ImGui_MyObjectA*");
  EXPECT_EQ(TypeInfo<MyObjectB   *>::type(), "ChunkyBacon*");
  EXPECT_EQ(TypeInfo<reaper_array*>::type(), "reaper_array*");

  EXPECT_EQ(TypeInfo<MyObjectA   *>::name(), "_");
}

TEST(TypesTest, Tags) {
  EXPECT_EQ(TypeInfo<W<int*    >>::type(), "int*");
  EXPECT_EQ(TypeInfo<W<double* >>::type(), "double*");

  EXPECT_EQ(TypeInfo<RO  <int*>>::name(), "_InOptional");
  EXPECT_EQ(TypeInfo<RW  <int*>>::name(), "_InOut");
  EXPECT_EQ(TypeInfo<RWO <int*>>::name(), "_InOutOptional");
  EXPECT_EQ(TypeInfo<W   <int*>>::name(), "_Out");
  EXPECT_EQ(TypeInfo<WS  <int*>>::name(), "_Out_sz");
  EXPECT_EQ(TypeInfo<RWB <int*>>::name(), "_InOutNeedBig");
  EXPECT_EQ(TypeInfo<RWBS<int*>>::name(), "_InOutNeedBig_sz");
  EXPECT_EQ(TypeInfo<WB  <int*>>::name(), "_OutNeedBig");
  EXPECT_EQ(TypeInfo<WBS <int*>>::name(), "_OutNeedBig_sz");
  EXPECT_EQ(TypeInfo<S   <int*>>::name(), "__sz");
}
