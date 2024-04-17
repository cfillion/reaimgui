#include "../src/vernum.hpp"

#include <gtest/gtest.h>

TEST(VerNumTest, ParseString) {
  EXPECT_EQ(VerNum { "1"       }, 0x01'00'00'00u);
  EXPECT_EQ(VerNum { "1.2"     }, 0x01'02'00'00u);
  EXPECT_EQ(VerNum { "1.2.3"   }, 0x01'02'03'00u);
  EXPECT_EQ(VerNum { "1.2.3.4" }, 0x01'02'03'04u);
  EXPECT_EQ(VerNum { "016.032.064.128" }, 0x10'20'40'80u);
  EXPECT_EQ(VerNum { "255.255.255.255" }, 0xFF'FF'FF'FFu);
}

TEST(VerNumTest, ValidateString) {
  EXPECT_THROW(VerNum { ""    }, reascript_error);
  EXPECT_THROW(VerNum { "."   }, reascript_error);
  EXPECT_THROW(VerNum { "1."  }, reascript_error);
  EXPECT_THROW(VerNum { "-1"  }, reascript_error);
  EXPECT_THROW(VerNum { "256" }, reascript_error);
  EXPECT_THROW(VerNum { "1.2.3.4.5" }, reascript_error);
}

TEST(VerNumTest, FormatString) {
  EXPECT_EQ(VerNum { "0"       }.toString(), "0.0");
  EXPECT_EQ(VerNum { "1"       }.toString(), "1.0");
  EXPECT_EQ(VerNum { "1.2.0.0" }.toString(), "1.2");
  EXPECT_EQ(VerNum { "1.2.3.0" }.toString(), "1.2.3");
  EXPECT_EQ(VerNum { "1.2.3.4" }.toString(), "1.2.3.4");
  EXPECT_EQ(VerNum { "016.032.064.128" }.toString(), "16.32.64.128");
}

static void expectSegments(const VerNum &ver,
  const std::array<decltype(ver[0]), 4> &segs)
{
  for(size_t i {}; i < segs.size(); ++i)
    EXPECT_EQ(ver[i], segs[i]);
}

TEST(VerNumTest, SegmentAccess) {
 expectSegments("0",     { 0, 0, 0, 0 });
 expectSegments("1",     { 1, 0, 0, 0 });
 expectSegments("1.2",   { 1, 2, 0, 0 });
 expectSegments("1.2.3", { 1, 2, 3, 0 });
 expectSegments("016.032.064.128", { 16, 32, 64, 128 });
}
