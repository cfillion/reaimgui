#include "../src/color.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <imgui/imgui.h>

static bool operator==(const ImVec4 &a, const ImVec4 &b)
{
  return a.x == b.x &&
         a.y == b.y &&
         a.z == b.z &&
         a.w == b.w;
}

static std::ostream &operator<<(std::ostream &stream, const ImVec4 &color)
{
  stream << "{ "
         << color.x << "f, " << color.y << "f, " << color.z << "f, " << color.w
         << "f }";
  return stream;
}

TEST_CASE("endianness conversion") {
  SECTION("big to little") {
    REQUIRE(Color::fromBigEndian(0x1a2b3c4d) == 0x4d3c2b1a);
  }

  SECTION("little to big") {
    REQUIRE(Color::toBigEndian(0x4d3c2b1a) == 0x1a2b3c4d);
  }
}

TEST_CASE("native RGB byte order") {
#ifdef _WIN32
  REQUIRE(Color::convertNative(0xdd112233) == 0xdd332211);
#else
  REQUIRE(Color::convertNative(0xdd112233) == 0xdd112233);
#endif
}

TEST_CASE("Color to ImVec4") {
  const bool withAlpha { GENERATE(true, false) };
  CAPTURE(withAlpha);

  SECTION("from ImVec4") {
    const Color color { ImVec4 { 0.2f, 0.4f, 0.6f, 0.8f }, withAlpha };
    REQUIRE(color == ImVec4 { 0.2f, 0.4f, 0.6f, withAlpha ? 0.8f : 1.0f });
  }

  SECTION("from float[]") {
    float arr[] { 0.2f, 0.4f, 0.6f, 0.8f };
    const Color color { arr, withAlpha };
    REQUIRE(color == ImVec4{ 0.2f, 0.4f, 0.6f, withAlpha ? 0.8f : 1.0f });
  }

  SECTION("from uint32_t") {
    const uint32_t packed { withAlpha ? 0x336699ccu : 0x336699u };
    const Color color { packed, withAlpha };
    REQUIRE(color == ImVec4 { 0.2f, 0.4f, 0.6f, withAlpha ? 0.8f : 1.0f });
  }
}

TEST_CASE("Color to float[]") {
  const ImVec4 expected { 0.256f, 0.512f, 0.1024f, 0.4096f };
  const Color color { expected };
  float actual[4];
  color.unpack(actual);
  REQUIRE(expected == ImVec4 { actual[0], actual[1], actual[2], actual[3] });
}

TEST_CASE("Color to uint32_t") {
  const bool withAlpha { GENERATE(true, false) };
  CAPTURE(withAlpha);

  const Color color { 0x11223344 };

  SECTION("preserve MSB") {
    if(withAlpha)
      REQUIRE(color.pack(true, 0xaabbccdd) == 0x11223344);
    else
      REQUIRE(color.pack(false, 0xaabbccdd) == 0xaa112233);
  }

  SECTION("without extra data") {
    if(withAlpha)
      REQUIRE(color.pack(true) == 0x11223344);
    else
      REQUIRE(color.pack(false) == 0x00112233);
  }
}

TEST_CASE("apply color to function") {
  ImVec4 expected { 0.1f, 0.2f, 0.3f, 0.4f };
  const Color color { expected };
  ImVec4 actual {};
  const auto retval {
    color.apply([&] (float r, float g, float b, float a) {
      actual = { r, g, b, a };
      return 42;
    })
  };
  REQUIRE(retval == 42);
  REQUIRE(actual == expected);
}
