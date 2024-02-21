#include "../src/resource_proxy.hpp"

#include <gtest/gtest.h>

struct MyResource : Resource {
  bool attachable(const Context *) const override { return false; }
  bool isValid() const override { return valid; }

  bool valid { true };
};

struct MyValue { Resource *res; unsigned int val; };

struct MyProxy : ResourceProxy<MyProxy, MyResource, MyValue> {
  struct TypeA {
    static constexpr Key key { 0x1234 };
    static MyValue *get(MyResource *res)
    {
      static MyValue value;
      value.res = res, value.val = key;
      return &value;
    }
  };

  struct TypeB {
    static constexpr Key key { 0x5678 };
    static MyValue *get(MyResource *res)
    {
      static MyValue value;
      value.res = res, value.val = key;
      return &value;
    }
  };

  using Decoder = MakeDecoder<TypeA, TypeB>;
};
API_REGISTER_BASIC_TYPE(MyProxy*);

TEST(ResourceProxyTest, Encode) {
  auto res { std::make_unique<MyResource>() };

  MyProxy *proxy1 { MyProxy::encode<MyProxy::TypeA>(res.get()) };
  EXPECT_EQ(reinterpret_cast<uintptr_t>(proxy1),
    reinterpret_cast<uintptr_t>(res.get()) ^ MyProxy::TypeA::key);

  MyProxy *proxy2 { MyProxy::encode<MyProxy::TypeB>(res.get()) };
  EXPECT_EQ(reinterpret_cast<uintptr_t>(proxy2),
    reinterpret_cast<uintptr_t>(res.get()) ^ MyProxy::TypeB::key);
}

TEST(ResourceProxyTest, Get) {
  auto res { std::make_unique<MyResource>() };

  MyProxy *proxy1 { reinterpret_cast<MyProxy *>(
    reinterpret_cast<uintptr_t>(res.get()) ^ MyProxy::TypeA::key) };
  MyResource *resPtr {};
  MyValue *val1 { proxy1->get(&resPtr) };
  EXPECT_EQ(resPtr, res.get());
  EXPECT_EQ(val1->res, res.get());
  EXPECT_EQ(val1->val, MyProxy::TypeA::key);

  MyProxy *proxy2 { reinterpret_cast<MyProxy *>(
    reinterpret_cast<uintptr_t>(res.get()) ^ MyProxy::TypeB::key) };
  MyValue *val2 { proxy2->get() };
  EXPECT_NE(val1, val2);
  EXPECT_EQ(val2->res, res.get());
  EXPECT_EQ(val2->val, MyProxy::TypeB::key);

  MyProxy *proxy3 { reinterpret_cast<MyProxy *>(
    reinterpret_cast<uintptr_t>(res.get()) ^ 0x90AB) };
  EXPECT_THROW(proxy3->get(), reascript_error);
}

TEST(ResourceProxyTest, Validate) {
  auto res { std::make_unique<MyResource>() };

  MyProxy *proxy1 { reinterpret_cast<MyProxy *>(
    reinterpret_cast<uintptr_t>(res.get()) ^ MyProxy::TypeA::key) };
  EXPECT_TRUE(proxy1->isValid());

  MyProxy *proxy2 { reinterpret_cast<MyProxy *>(
    reinterpret_cast<uintptr_t>(res.get()) ^ MyProxy::TypeB::key) };
  EXPECT_TRUE(proxy2->isValid());

  MyProxy *proxy3 { reinterpret_cast<MyProxy *>(
    reinterpret_cast<uintptr_t>(res.get()) ^ 0x90AB) };
  EXPECT_FALSE(proxy3->isValid());
}
