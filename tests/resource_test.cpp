#include "../src/resource.hpp"

#include <gtest/gtest.h>

struct Foo : Resource {
  bool attachable(const Context *) const override { return false; }
  bool isValid() const override { return valid; }

  bool valid { true };
};

struct Bar : Foo {};

struct Baz : Resource {
  bool attachable(const Context *) const override { return false; }
};

struct Lifetime : Resource {
  Lifetime(bool *alive) : m_alive { alive } { *m_alive = true; }
  virtual ~Lifetime() { *m_alive = false; }
  bool attachable(const Context *) const override { return false; }

  bool *m_alive;
};

TEST(ResourceTest, ValidateNull) {
  auto foo { std::make_unique<Foo>() };
  EXPECT_FALSE(Resource::isValid<Foo>(nullptr));
}

TEST(ResourceTest, ValidateExists) {
  auto foo { std::make_unique<Foo>() };
  EXPECT_TRUE(Resource::isValid<Foo>(foo.get()));
  foo->valid = false;
  EXPECT_FALSE(Resource::isValid<Foo>(foo.get()));
}

TEST(ResourceTest, ValidateDangling) {
  Foo *foo { new Foo };
  delete foo;
  EXPECT_FALSE(Resource::isValid<Foo>(foo));
}

TEST(ResourceTest, ValidateChild) {
  auto foo { std::make_unique<Foo>() };
  EXPECT_FALSE(Resource::isValid<Bar>(static_cast<Bar *>(foo.get())));
}

TEST(ResourceTest, ValidateParent) {
  auto bar { std::make_unique<Bar>() };
  EXPECT_TRUE(Resource::isValid<Foo>(bar.get()));
  bar->valid = false;
  EXPECT_FALSE(Resource::isValid<Foo>(bar.get()));
}

TEST(ResourceTest, ValidateBaseType) {
  auto foo { std::make_unique<Foo>() };
  EXPECT_TRUE(Resource::isValid<Resource>(foo.get()));
  foo->valid = false;
  EXPECT_FALSE(Resource::isValid<Resource>(foo.get()));
}

TEST(ResourceTest, ValidateVoid) {
  auto foo { std::make_unique<Foo>() };
  EXPECT_TRUE(Resource::isValid<void>(foo.get()));
  foo->valid = false;
  EXPECT_FALSE(Resource::isValid<void>(foo.get()));
}

TEST(ResourceTest, ForeachBase) {
  Foo foo; Bar bar; Baz baz;

  unsigned int matches {};
  Resource::foreach<Resource>([&matches](Resource *) { ++matches; });
  EXPECT_EQ(matches, 3u);
}

TEST(ResourceTest, ForeachDerived) {
  Foo foo; Bar bar; Baz baz;

  unsigned int matches {};
  Resource::foreach<Foo>([&matches](const Foo *) { ++matches; });
  EXPECT_EQ(matches, 2u); // Foo + Bar (derived from Foo)
}

TEST(ResourceTest, KeepAlive) {
  bool alive { false };
  Lifetime res { &alive };
  EXPECT_TRUE(alive);
  for(int i {}; i < 1024; ++i) {
    res.keepAlive();
    Resource::testHeartbeat();
    ASSERT_TRUE(alive);
  }
}

TEST(ResourceTest, GarbageCollection) {
  bool alive { false };
  auto res = new Lifetime { &alive };
  EXPECT_TRUE(alive);
  for(int i {}; i < 2; ++i) {
    EXPECT_TRUE(Resource::isValid(res));
    Resource::testHeartbeat();
    ASSERT_TRUE(alive);
  }
  EXPECT_FALSE(Resource::isValid(res));
  Resource::testHeartbeat();
  EXPECT_FALSE(alive);
}
