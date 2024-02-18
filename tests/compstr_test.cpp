#include "../api/compstr.hpp"

#include <gtest/gtest.h>

constexpr const char
  g_foo[] { "foo" },
  g_ext[] { "foo.bar" },
  g_dir[] { "foo/bar" },
  g_dot[] { "foo.bar/baz" },
  g_abs[] { "/lorem/ipsum/chunky.bacon.cpp" };

TEST(CompStrTest, Basename) {
  EXPECT_STREQ(CompStr::basename<&g_foo>, "foo");
  EXPECT_STREQ(CompStr::basename<&g_ext>, "foo");
  EXPECT_STREQ(CompStr::basename<&g_dir>, "bar");
  EXPECT_STREQ(CompStr::basename<&g_dot>, "baz");
  EXPECT_STREQ(CompStr::basename<&g_abs>, "chunky.bacon");
}

constexpr const char
  g_underscores[] { "1_20_3" },
  g_gitdescribe[] { "v0.8.7.4-41-g6d2c9f5" },
  g_return_asis[] { "0.8.7" };

TEST(CompStrTest, Version) {
  EXPECT_STREQ(CompStr::version<&g_underscores>, "1.20.3");
  EXPECT_STREQ(CompStr::version<&g_gitdescribe>, "0.8.7.4");
  EXPECT_STREQ(CompStr::version<&g_return_asis>, "0.8.7");
}
