#include <../src/api.hpp>

#include <gtest/gtest.h>

using namespace API;

class MyCallable : public Callable {
public:
  MyCallable(VerNum a, VerNum b, const char *n, uintptr_t safe, uintptr_t unsafe)
    : Callable { a, b, n },
      m_safe   { reinterpret_cast<void *>(safe)   },
      m_unsafe { reinterpret_cast<void *>(unsafe) }
  {}
  virtual void *safeImpl()   const override { return m_safe;   }
  virtual void *unsafeImpl() const override { return m_unsafe; }

private:
  void *m_safe, *m_unsafe;
};

TEST(APITest, LookupCallable) {
  MyCallable foo1 { "0.7", "0.8", "test!lookup!foo", 1, 2 };
  MyCallable foo2 { "0.8", "0.9", "test!lookup!foo", 3, 4 };
  MyCallable bar  { "0.8", VerNum::MAX, "test!lookup!bar", 5, 6 };

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

TEST(APITest, ImportTable) {
  struct MyImport : ImportTable {
    MyImport() : ImportTable { "0.8.5", sizeof(*this) } {}
    void *foo = const_cast<char *>("test!import!foo");
    void *bar = const_cast<char *>("test!import!bar");
  } table;

  MyCallable foo1 { "0.7", "0.8", "test!import!foo", 1, 2 };
  MyCallable foo2 { "0.8", "0.9", "test!import!foo", 3, 4 };
  MyCallable foo3 { "1.0", "2.0", "test!import!foo", 5, 6 };
  MyCallable bar  { "0.8", VerNum::MAX, "test!import!bar", 7, 8 };

  table.resolve();

  EXPECT_EQ(table.foo, foo2.unsafeImpl());
  EXPECT_EQ(table.bar, bar.unsafeImpl());
}
