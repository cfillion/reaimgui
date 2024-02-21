#include "../api/compstr.hpp"

#include <gtest/gtest.h>

TEST(CompStrTest, Basename) {
  static constexpr const char
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
  static constexpr const char
    g_underscores[] { "1_20_3" },
    g_gitdescribe[] { "v0.8.7.4-41-g6d2c9f5" },
    g_return_asis[] { "0.8.7" };

  EXPECT_STREQ(CompStr::version<&g_underscores>, "1.20.3");
  EXPECT_STREQ(CompStr::version<&g_gitdescribe>, "0.8.7.4");
  EXPECT_STREQ(CompStr::version<&g_return_asis>, "0.8.7");
}

static int func1(reaper_array *, int, W<double*>) { return 0; }

TEST(CompStrTest, APIDef) {
  using namespace std::literals;

  constexpr std::string_view def1 {
    CompStr::APIDef<&func1>::value.data(),
    CompStr::APIDef<&func1>::value.size()
  };
  EXPECT_EQ(def1,
    "int\0reaper_array*,int,double*\0_,_,_Out\0Internal use only.\0"sv);
}
