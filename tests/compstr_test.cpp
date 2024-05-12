#include "../api/compstr.hpp"

#include <gtest/gtest.h>

TEST(CompStrTest, Basename) {
  static constexpr char
    g_foo[] { "foo" },
    g_ext[] { "foo.bar" },
    g_dir[] { "foo/bar" },
    g_dot[] { "foo.bar/baz" },
    g_abs[] { "/lorem/ipsum/chunky.bacon.cpp" };

  EXPECT_STREQ(CompStr::basename<&g_foo>, "foo");
  EXPECT_STREQ(CompStr::basename<&g_ext>, "foo");
  EXPECT_STREQ(CompStr::basename<&g_dir>, "bar");
  EXPECT_STREQ(CompStr::basename<&g_dot>, "baz");
  EXPECT_STREQ(CompStr::basename<&g_abs>, "chunky.bacon");
}

TEST(CompStrTest, Version) {
  static constexpr char
    g_underscores[] { "1_20_3" },
    g_gitdescribe[] { "v0.8.7.4-41-g6d2c9f5" },
    g_dirtygittag[] { "v0.9+" },
    g_return_asis[] { "0.8.7" };

  EXPECT_STREQ(CompStr::version<&g_underscores>, "1.20.3");
  EXPECT_STREQ(CompStr::version<&g_gitdescribe>, "0.8.7.4");
  EXPECT_STREQ(CompStr::version<&g_dirtygittag>, "0.9");
  EXPECT_STREQ(CompStr::version<&g_return_asis>, "0.8.7");
}

using F1 = int (*)(reaper_array *, int, W<double*>);
struct F1_Meta {
  static constexpr std::string_view help { "Lorem ipsum" };
  static constexpr std::array<std::string_view, 3> argn
    { "foo", "bar", "baz" };
};
using F2 = void (*)();
struct F2_Meta {
  static constexpr std::string_view help { "Lorem ipsum" };
  static constexpr std::array<std::string_view, 0> argn {};
};

TEST(CompStrTest, APIDef) {
  using namespace std::string_view_literals;

  const std::string_view def1 {
    CompStr::APIDef<F1, F1_Meta>::value.data(),
    CompStr::APIDef<F1, F1_Meta>::value.size()
  };
  EXPECT_EQ(def1,
    "int\0reaper_array*,int,double*\0foo,bar,bazOut\0Lorem ipsum\0"sv);

  const std::string_view def1_anon {
    CompStr::APIDef<F1, F1_Meta, false>::value.data(),
    CompStr::APIDef<F1, F1_Meta, false>::value.size()
  };
  EXPECT_EQ(def1_anon,
    "int\0reaper_array*,int,double*\0_,_,_Out\0Lorem ipsum\0"sv);

  const std::string_view def2 {
    CompStr::APIDef<F2, F2_Meta>::value.data(),
    CompStr::APIDef<F2, F2_Meta>::value.size()
  };
  EXPECT_EQ(def2, "void\0\0\0Lorem ipsum\0"sv);
}
