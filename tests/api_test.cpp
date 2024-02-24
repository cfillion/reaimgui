#include <../src/api.hpp>

#include <gtest/gtest.h>

using namespace API;

class MyCallable : public Callable {
public:
  MyCallable(VerNum since, VerNum until, const char *name,
    uintptr_t safe = 0, uintptr_t unsafe = 0)
    : Callable { since, until, name },
      m_safe   { reinterpret_cast<void *>(safe)   },
      m_unsafe { reinterpret_cast<void *>(unsafe) }
  {}

  void *safeImpl()   const override { return m_safe;   }
  void *unsafeImpl() const override { return m_unsafe; }
  bool  isConstant() const override { return false;    }

private:
  void *m_safe, *m_unsafe;
};

TEST(APITest, FuncFlags) {
  const ReaScriptFunc func { "0.9", nullptr,
    { "-API_"       API_PREFIX "foo", nullptr },
    { "-APIvararg_" API_PREFIX "foo", nullptr },
    { "-APIdef_"    API_PREFIX "foo",
      const_cast<char *>("int\0ImGui_Context*\0ctx\0help text") },
  };
  EXPECT_STREQ(func.name(), "foo");
  EXPECT_EQ(func.m_flags, API::Symbol::TargetNative | API::Symbol::TargetScript);

  const ReaScriptFunc cnst { "0.9", nullptr,
    { "-API_"       API_PREFIX "bar", nullptr },
    { "-APIvararg_" API_PREFIX "bar", nullptr },
    { "-APIdef_"    API_PREFIX "bar",
      const_cast<char *>("int\0\0\0help text") },
  };
  EXPECT_TRUE(cnst.m_flags & API::Symbol::Constant);

  const ReaScriptFunc factory { "0.9", nullptr,
    { "-API_"       API_PREFIX "create", nullptr },
    { "-APIvararg_" API_PREFIX "create", nullptr },
    { "-APIdef_"    API_PREFIX "create",
      const_cast<char *>("ImGui_ImageSet*\0\0\0help text") },
  };
  EXPECT_FALSE(factory.m_flags & API::Symbol::Constant);
}

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

TEST(APITest, RollbackCallable) {
  MyCallable foo1 { "0.5", "0.7", "test!rollback!foo" };
  EXPECT_EQ(foo1.rollback("0.4"), nullptr);
  EXPECT_EQ(foo1.rollback("0.5"), &foo1);
  EXPECT_EQ(foo1.rollback("0.6"), &foo1);
  EXPECT_EQ(foo1.rollback("0.7"), nullptr);
  EXPECT_EQ(foo1.rollback("0.8"), nullptr);

  MyCallable foo2 { "0.7", "0.8", "test!rollback!foo" };
  EXPECT_EQ(foo2.rollback("0.6"), &foo1);
  EXPECT_EQ(foo2.rollback("0.7"), &foo2);
  EXPECT_EQ(foo2.rollback("0.8"), nullptr);
  EXPECT_EQ(foo1.rollback("0.7"), nullptr);

  // out-of-order initialization
  MyCallable foo0 { "0.4", "0.5", "test!rollback!foo" };
  EXPECT_EQ(foo2.rollback("0.4"), &foo0);
  EXPECT_EQ(foo2.rollback("0.5"), &foo1);
  EXPECT_EQ(foo1.rollback("0.4"), &foo0);
}

TEST(APITest, OverlappingCallable) {
  MyCallable foo { "0.5", "0.7", "test!overlap!foo" };
  EXPECT_THROW((MyCallable { "0.4", "0.6", "test!overlap!foo" }), reascript_error);
  EXPECT_THROW((MyCallable { "0.6", "0.8", "test!overlap!foo" }), reascript_error);
  EXPECT_THROW((MyCallable { "0.5", "0.7", "test!overlap!foo" }), reascript_error);
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
