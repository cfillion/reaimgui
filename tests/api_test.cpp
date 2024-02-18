#include <../src/api.hpp>

#include <gtest/gtest.h>

using namespace API;

class MyCallable : public Callable {
public:
  MyCallable(VerNum a, VerNum b, const char *n)
    : Callable { a, b, n }
  {}
};

TEST(APITest, LookupCallable) {
  MyCallable foo1 { "0.7", "0.8", "test!lookup!foo" };
  MyCallable foo2 { "0.8", "0.9", "test!lookup!foo" };
  MyCallable bar  { "0.8", VerNum::MAX, "test!lookup!bar" };

  EXPECT_EQ(MyCallable::lookup("0.6.0", "test!lookup!foo"), nullptr);
  EXPECT_EQ(MyCallable::lookup("0.6.1", "test!lookup!foo"), nullptr);
  EXPECT_EQ(MyCallable::lookup("0.7.0", "test!lookup!foo"), &foo1);
  EXPECT_EQ(MyCallable::lookup("0.7.1", "test!lookup!foo"), &foo1);
  EXPECT_EQ(MyCallable::lookup("0.8.0", "test!lookup!foo"), &foo2);
  EXPECT_EQ(MyCallable::lookup("0.8.1", "test!lookup!foo"), &foo2);
  EXPECT_EQ(MyCallable::lookup("0.9.0", "test!lookup!foo"), nullptr);

  EXPECT_EQ(MyCallable::lookup("0.7.0", "test!lookup!bar"), nullptr);
  EXPECT_EQ(MyCallable::lookup("0.8.0", "test!lookup!bar"), &bar);
  EXPECT_EQ(MyCallable::lookup("0.9.0", "test!lookup!bar"), &bar);
}
