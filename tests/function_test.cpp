#include "../src/function.hpp"

#include "../src/api_eel.hpp"
#include "../src/error.hpp"

#include <eel2/ns-eel.h> // NSEEL_RAM_ITEMSPERBLOCK
#include <gtest/gtest.h>
#include <numeric>
#include <reaper_plugin_secrets.h>

using namespace std::string_view_literals;

constexpr const char *NO_CODE { "0;" };

TEST(FunctionTest, Invalid) {
  EXPECT_THROW({ Function func { "(" }; }, reascript_error);
}

TEST(FunctionTest, Double404) {
  Function func { NO_CODE };
  EXPECT_FALSE(func.getDouble("output"));
}

TEST(FunctionTest, GetSetUnused) {
  Function func { NO_CODE };
  EXPECT_TRUE(func.setDouble("foo", 3.14));
  EXPECT_EQ(func.getDouble("foo"), 3.14);
}

TEST(FunctionTest, DoubleMath) {
  Function func { "output = input * 2;" };
  EXPECT_TRUE(func.setDouble("input", 2.1));
  func.execute();
  const auto &output { func.getDouble("output") };
  ASSERT_TRUE(output);
  EXPECT_EQ(*output, 4.2);
}

TEST(FunctionTest, DoubleFunc) {
  Function func { "output = floor(3.14);" };
  func.execute();
  const auto &output { func.getDouble("output") };
  ASSERT_TRUE(output);
  EXPECT_EQ(*output, 3.0);
}

TEST(FunctionTest, InitialValue) {
  Function func { "output = 6.28;" };
  const auto &output { func.getDouble("output") };
  ASSERT_TRUE(output);
  EXPECT_EQ(*output, 0);
}

TEST(FunctionTest, DoubleLeadingHash) {
  // EEL impl detail: variables with leading '#' are reserved for strings
  Function func { NO_CODE };
  EXPECT_FALSE(func.setDouble("#foo", 3.14));
  EXPECT_FALSE(func.getDouble("#foo"));
}

TEST(FunctionTest, String404) {
  Function func { NO_CODE };
  EXPECT_FALSE(func.getString("output"));
  EXPECT_FALSE(func.getString("#output"));
}

TEST(FunctionTest, StringOperator) {
  Function func { R"(#output = "hello "; #output += #input;)" };
  EXPECT_TRUE(func.setString("#input", "world"));
  func.execute();
  const auto &output { func.getString("#output") };
  ASSERT_TRUE(output);
  EXPECT_EQ(*output, "hello world"sv);
}

TEST(FunctionTest, StringFunc) {
  Function func { R"(strcpy(#output, "hello "); strcat(#output, #input);)" };
  EXPECT_TRUE(func.setString("#input", "world"));
  func.execute();
  const auto &output { func.getString("#output") };
  ASSERT_TRUE(output);
  EXPECT_EQ(*output, "hello world"sv);
}

TEST(FunctionTest, StringBinarySafe) {
  const std::string_view input { "lorem\0ipsum"sv };
  Function func { R"(inputlen = strlen(#input); #output = "chunky\0bacon";)" };
  func.setString("#input", input);
  func.execute();
  EXPECT_EQ(*func.getDouble("inputlen"), input.size());
  EXPECT_EQ(*func.getString("#output"), "chunky\0bacon"sv);
}

TEST(FunctionTest, UnnamedString) {
  Function func { R"(inputlen = strlen(input); output = "foobar";)" };
  func.setDouble("input", 123);
  EXPECT_TRUE(func.setString("input", "chunky bacon"));
  func.execute();
  EXPECT_EQ(*func.getDouble("inputlen"), 12);

  const auto &output { func.getString("output") };
  ASSERT_TRUE(output);
  EXPECT_EQ(*output, "foobar"sv);
}

TEST(FunctionTest, UnnamedStringOperation) {
  Function func { R"(strcpy(output, "hello "); strcat(output, input);)" };
  func.setDouble("output", 0);
  func.setDouble("input",  1);
  EXPECT_TRUE(func.setString("input", "world"));
  func.execute();

  const auto &output { func.getString("output") };
  ASSERT_TRUE(output);
  EXPECT_EQ(*output, "hello world"sv);
}

TEST(FunctionTest, StringSlot) {
  Function func { R"(unnamed = "foo"; #named = "bar";)" };
  const auto &unnamed { func.getDouble("unnamed") };
  ASSERT_TRUE(unnamed);
  EXPECT_EQ(*unnamed, 0);

  const auto &named { func.getDouble("#named") };
  ASSERT_TRUE(named);
  EXPECT_EQ(*named, 90'000);

  EXPECT_FALSE(func.setDouble("#named", 42));
  EXPECT_FALSE(func.setDouble("#other", 23));
}

TEST(FunctionTest, ErrorHandling) {
  static bool ok;
  ReaScriptError = [](const char *msg)
  {
    ok = !strcmp(msg, "sprintf: bad format string %🐈");
  };

  Function func { R"(sprintf(1, "%🐈"))" };
  ok = false;
  func.execute();
  EXPECT_TRUE(ok);
}

TEST(FunctionTest, Array404) {
  double mem[0xF];
  memset(mem, 0, sizeof(mem));
  reaper_array *values { reinterpret_cast<reaper_array *>(&mem) };

  Function func { NO_CODE };
  EXPECT_FALSE(func.getArray("unset_var", values));
  EXPECT_FALSE(func.setArray("unset_var", values));
}

static double arraySlot(const reaper_array *values)
{
  // give a slot where the array will be split over two blocks
  return NSEEL_RAM_ITEMSPERBLOCK - (values->size / 2);
}

TEST(FunctionTest, ReadArray) {
  double mem[0xF];
  reaper_array *values { reinterpret_cast<reaper_array *>(&mem) };
  *const_cast<unsigned int *>(&values->size) = 8;
  std::fill(values->data, values->data + values->size, 0);

  const double slot { arraySlot(values) };
  Function func { "a[0]=1;a[1]=2;a[2]=3;a[3]=4;a[4]=5;a[5]=6;a[6]=7;a[7]=8;" };
  func.setDouble("a", slot);
  func.execute();
  EXPECT_TRUE(func.getArray("a", values));

  for(unsigned int i {}; i < values->size; ++i)
    ASSERT_EQ(values->data[i], i + 1);
}

TEST(FunctionTest, WriteArray) {
  double mem[0xF];
  reaper_array *values { reinterpret_cast<reaper_array *>(&mem) };
  *const_cast<unsigned int *>(&values->size) = 8;
  std::iota(values->data, values->data + values->size, 1);

  const double slot { arraySlot(values) };
  Function func { "r = a + a[0] + a[7];" };
  func.setDouble("a", slot);
  EXPECT_TRUE(func.setArray("a", values));
  func.execute();
  EXPECT_EQ(*func.getDouble("r"), slot + 1 + 8);
}

static double customFunc1() noexcept // testing handling of 0 arguments
{
  return 16;
}

static double customFunc2(const double rh, const double lh,
                         std::string_view str) noexcept
{
  return (rh * lh) + str.size();
}

TEST(FunctionTest, CustomFunction) {
  const API::EELFunc funcs[] {
    { "0.8.5", "MyFunc1", "double\0\0\0",
      &CallConv::EEL<&customFunc1>::apply, CallConv::EEL<&customFunc1>::ARGC,
    },
    { "0.8.5", "MyFunc2", "double\0double,double\0rh,lh\0",
      &CallConv::EEL<&customFunc2>::apply, CallConv::EEL<&customFunc2>::ARGC,
    },
  };

  for(const auto &func : funcs)
    func.announce(true);

  Function func { R"(rv = MyFunc2(2, MyFunc1(), "chunky\0bacon");)" };
  func.execute();
  EXPECT_EQ(*func.getDouble("rv"), 44);
}
