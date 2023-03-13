#include "../src/color.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <imgui/imgui.h>

class ColorTest : public testing::TestWithParam<bool> {};
INSTANTIATE_TEST_SUITE_P(Alpha, ColorTest, testing::Values(false, true));

static bool operator==(const ImVec4 &a, const ImVec4 &b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

static std::ostream &operator<<(std::ostream &stream, const ImVec4 &color)
{
  stream << "{ "
         << color.x << "f, " << color.y << "f, " << color.z << "f, " << color.w
         << "f }";
  return stream;
}

TEST(ColorTest, FromBigEndian) {
  EXPECT_EQ(Color::fromBigEndian(0x1a2b3c4d), 0x4d3c2b1aU);
}

TEST(ColorTest, ToBigEndian) {
  EXPECT_EQ(Color::toBigEndian(0x4d3c2b1a), 0x1a2b3c4dU);
}

TEST(ColorTest, ConvertNative) {
#ifdef _WIN32
  EXPECT_EQ(Color::convertNative(0xdd112233), 0xdd332211u);
#else
  EXPECT_EQ(Color::convertNative(0xdd112233), 0xdd112233u);
#endif
}

TEST_P(ColorTest, FromImVec4) {
  const bool alpha { GetParam() };
  const Color color { ImVec4 { 0.2f, 0.4f, 0.6f, 0.8f }, alpha };
  const ImVec4 expected { 0.2f, 0.4f, 0.6f, alpha ? 0.8f : 1.0f };
  EXPECT_EQ(color, expected);
}

TEST_P(ColorTest, FromFloatArray) {
  const bool alpha { GetParam() };
  float arr[] { 0.2f, 0.4f, 0.6f, 0.8f };
  const Color color { arr, alpha };
  const ImVec4 expected { 0.2f, 0.4f, 0.6f, alpha ? 0.8f : 1.0f };
  EXPECT_EQ(color, expected);
}

TEST_P(ColorTest, FromInteger) {
  const bool alpha { GetParam() };
  const uint32_t packed { alpha ? 0x336699ccu : 0x336699u };
  const Color color { packed, alpha };
  const ImVec4 expected { 0.2f, 0.4f, 0.6f, alpha ? 0.8f : 1.0f };
  EXPECT_EQ(color, expected);
}

TEST(ColorTest, ToFloatArray) {
  const Color color { ImVec4 { 0.256f, 0.512f, 0.1024f, 0.4096f } };
  float actual[4];
  color.unpack(actual);
  EXPECT_THAT(actual, testing::ElementsAre(0.256f, 0.512f, 0.1024f, 0.4096f));
}

TEST_P(ColorTest, ToInteger) {
  const Color color { 0x11223344 };
  if(GetParam()) // with alpha
    EXPECT_EQ(color.pack(true), 0x11223344U);
  else
    EXPECT_EQ(color.pack(false), 0x00112233U);
}

TEST_P(ColorTest, ToIntegerWithMSB) {
  const Color color { 0x11223344 };
  if(GetParam()) // with alpha
    EXPECT_EQ(color.pack(true, 0xaabbccdd), 0x11223344u);
  else
    EXPECT_EQ(color.pack(false, 0xaabbccdd), 0xaa112233u);
}

TEST(ColorTest, ApplyToFunction) {
  using namespace std::placeholders;
  using testing::Return;

  struct Mock {
    MOCK_METHOD(int, func, (float r, float g, float b, float a));
  } mock;

  const Color color { ImVec4 { 0.1f, 0.2f, 0.3f, 0.4f } };
  EXPECT_CALL(mock, func(0.1f, 0.2f, 0.3f, 0.4f)).WillOnce(Return(42));
  EXPECT_EQ(color.apply(std::bind(&Mock::func, &mock, _1, _2, _3, _4)), 42);
}
