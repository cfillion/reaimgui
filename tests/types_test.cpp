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

  static constexpr std::string_view names[] { "foo" };
  EXPECT_EQ((TypeInfo<int>::name<names, 0>()), "foo");
}

struct MyObjectA; struct MyObjectB;
API_REGISTER_OBJECT_TYPE(MyObjectA);
API_REGISTER_TYPE(MyObjectB*, "ChunkyBacon*");

TEST(TypesTest, Objects) {
  EXPECT_EQ(TypeInfo<MyObjectA   *>::type(), "ImGui_MyObjectA*");
  EXPECT_EQ(TypeInfo<MyObjectB   *>::type(), "ChunkyBacon*");
  EXPECT_EQ(TypeInfo<reaper_array*>::type(), "reaper_array*");

  static constexpr std::string_view names[] { "foo" };
  EXPECT_EQ((TypeInfo<MyObjectA   *>::name<names, 0>()), "foo");
}

TEST(TypesTest, Tags) {
  EXPECT_EQ(TypeInfo<W<int*    >>::type(), "int*");
  EXPECT_EQ(TypeInfo<W<double* >>::type(), "double*");

  static constexpr std::string_view names[]
    { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j" };
  EXPECT_EQ((TypeInfo<RO  <int*>>::name<names, 0>()), "aInOptional");
  EXPECT_EQ((TypeInfo<RW  <int*>>::name<names, 1>()), "bInOut");
  EXPECT_EQ((TypeInfo<RWO <int*>>::name<names, 2>()), "cInOutOptional");
  EXPECT_EQ((TypeInfo<W   <int*>>::name<names, 3>()), "dOut");
  EXPECT_EQ((TypeInfo<WS  <int*>>::name<names, 4>()), "eOut_sz");
  EXPECT_EQ((TypeInfo<RWB <int*>>::name<names, 5>()), "fInOutNeedBig");
  EXPECT_EQ((TypeInfo<RWBS<int*>>::name<names, 6>()), "gInOutNeedBig_sz");
  EXPECT_EQ((TypeInfo<WB  <int*>>::name<names, 7>()), "hOutNeedBig");
  EXPECT_EQ((TypeInfo<WBS <int*>>::name<names, 8>()), "iOutNeedBig_sz");
  EXPECT_EQ((TypeInfo<S   <int*>>::name<names, 9>()), "j_sz");
}
