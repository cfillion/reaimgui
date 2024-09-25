#include "../src/texture.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <imgui/imgui_internal.h>
#include <memory>

using CmdVector = std::vector<TextureCmd>;

class LogCmds {
public:
  LogCmds(CmdVector &log) : m_log { log } {}
  void operator()(const TextureCmd &cmd) { m_log.push_back(cmd); }

private:
  CmdVector &m_log;
};

static bool operator==(const TextureCmd &a, const TextureCmd &b)
{
  return a.manager == b.manager &&
         a.type    == b.type    &&
         a.offset  == b.offset  &&
         a.size    == b.size;
}

static std::ostream &operator<<(std::ostream &os, const TextureCmd &cmd)
{
  switch(cmd.type) {
  case TextureCmd::Insert: os << "Insert"; break;
  case TextureCmd::Update: os << "Update"; break;
  case TextureCmd::Remove: os << "Remove"; break;
  }
  os << '(' << cmd.offset << ", " << cmd.size << ')';
  return os;
}

TEST(TextureTest, TouchOutOfOrder) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  TextureManager manager;
  EXPECT_EQ(manager.touch((void *)0x10, 2.f, nullptr), 0u);
  EXPECT_EQ(manager.touch((void *)0x10, 1.f, nullptr), 1u);
  EXPECT_EQ(manager.touch((void *)0x10, 2.f, nullptr), 0u);
  EXPECT_EQ(manager.touch((void *)0x20, 1.f, nullptr), 2u);
}

TEST(TextureTest, InsertTail) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.touch((void *)0x50, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Insert, 0, 5 },
  }));
}

TEST(TextureTest, InsertMiddle) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.touch((void *)0x50, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.touch((void *)0x12, 1.f, nullptr);
  manager.touch((void *)0x15, 1.f, nullptr);
  manager.touch((void *)0x30, 2.f, nullptr);
  manager.touch((void *)0xff, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Insert, 5, 4 }, // 0x12 and 0x14
  }));
}

TEST(TextureTest, RemoveTail) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.remove((void *)0x20);
  manager.remove((void *)0x30);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Remove, 1, 2 },
  }));
}

TEST(TextureTest, RemoveMiddle) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.remove((void *)0x20);
  manager.remove((void *)0x30);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Remove, 1, 2 },
  }));
}

TEST(TextureTest, CleanupInactive) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };
  const ImGuiIO &io { ImGui::GetIO() };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.cleanup();
  manager.update(&cookie, LogCmds { cmds });
  ASSERT_THAT(cmds, testing::IsEmpty());

  ctx->Time += io.ConfigMemoryCompactTimer;
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.cleanup();
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Remove, 0, 2 }, // 0x10, 0x20
    { &manager, TextureCmd::Remove, 1, 1 }, // 0x40
  }));
}

TEST(TextureTest, CleanupInvalid) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr, [](void *object) {
    EXPECT_EQ(object, (void *)0x10);
    return false;
  });
  manager.touch((void *)0x20, 1.f, nullptr, [](void *object) {
    EXPECT_EQ(object, (void *)0x20);
    return true;
  });
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.cleanup();
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Remove, 0, 1 },
  }));
}

TEST(TextureTest, CleanupForceNoCompact) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };
  const ImGuiIO &io { ImGui::GetIO() };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr, nullptr,
    [](const Texture &tex) {
      EXPECT_EQ(tex.object(), (void *)0x10);
      EXPECT_EQ(tex.scale(), 1.f);
      return false; // prevent the texture from being removed
    });
  manager.touch((void *)0x20, 1.5f, nullptr, nullptr,
    [](const Texture &tex) {
      EXPECT_EQ(tex.object(), (void *)0x20);
      EXPECT_EQ(tex.scale(), 1.5f);
      return true; // accept compact request
    });
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  ctx->Time += io.ConfigMemoryCompactTimer;
  manager.cleanup();
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Remove, 1, 1 },
  }));
}

TEST(TextureTest, CleanupRecomputeIndices) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };
  const ImGuiIO &io { ImGui::GetIO() };

  TextureManager manager;

  // out of order to test whether indices are resorted
  EXPECT_EQ(manager.touch((void *)0x40, 1.f, nullptr), 0u);
  EXPECT_EQ(manager.touch((void *)0x30, 1.f, nullptr), 1u);
  EXPECT_EQ(manager.touch((void *)0x20, 1.f, nullptr), 2u);
  EXPECT_EQ(manager.touch((void *)0x10, 1.f, nullptr), 3u);

  ctx->Time += io.ConfigMemoryCompactTimer;
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.cleanup();

  EXPECT_EQ(manager.touch((void *)0x10, 1.f, nullptr), 2u);
  EXPECT_EQ(manager.touch((void *)0x20, 1.f, nullptr), 3u);
  EXPECT_EQ(manager.touch((void *)0x30, 1.f, nullptr), 1u);
  EXPECT_EQ(manager.touch((void *)0x40, 1.f, nullptr), 0u);
}

TEST(TextureTest, Update) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x30, 2.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.invalidate((void *)0x10);
  manager.invalidate((void *)0x30);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Update, 0, 1 },
    { &manager, TextureCmd::Update, 2, 2 }, // scale=1.f and 2.f
  }));
}

TEST(TextureTest, InsertAfterRemove) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x1, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.remove((void *)0x1);
  manager.touch((void *)0x2, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Remove, 0, 1 },
    { &manager, TextureCmd::Insert, 0, 1 },
  }));
}

TEST(TextureTest, InsertRemoveInsert) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x20, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.remove((void *)0x20);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Remove, 0, 1 },
    { &manager, TextureCmd::Insert, 0, 2 },
  }));
}

TEST(TextureTest, UpdateRemoveUpdate) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.touch((void *)0x50, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.invalidate((void *)0x10);
  manager.remove((void *)0x20);
  manager.remove((void *)0x30);
  manager.invalidate((void *)0x50);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Update, 0, 1 },
    { &manager, TextureCmd::Remove, 1, 2 },
    { &manager, TextureCmd::Update, 2, 1 },
  }));
}

TEST(TextureTest, UpdateOnReinsertion) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  CmdVector      cmds;
  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x1, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  cmds.clear();

  manager.remove((void *)0x1);
  manager.touch((void *)0x1, 1.f, nullptr);
  manager.update(&cookie, LogCmds { cmds });
  EXPECT_THAT(cmds, testing::ElementsAreArray(CmdVector {
    { &manager, TextureCmd::Update, 0, 1 },
  }));
}
