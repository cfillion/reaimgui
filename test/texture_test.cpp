#include "../src/texture.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <imgui/imgui.h>
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

TEST(TextureTest, OutOfOrderInsertion) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  TextureManager manager;
  manager.touch((void *)0x10, 1.75f, nullptr);
  EXPECT_EQ(manager.touch((void *)0x10, 1.f,   nullptr), 0);
  EXPECT_EQ(manager.touch((void *)0x10, 1.75f, nullptr), 1);
}

TEST(TextureTest, UpdateCommands) {
  std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> ctx
    { ImGui::CreateContext(), &ImGui::DestroyContext };

  TextureManager manager;
  TextureCookie  cookie;

  manager.touch((void *)0x10, 1.f, nullptr);
  manager.touch((void *)0x20, 1.f, nullptr);
  manager.touch((void *)0x30, 1.f, nullptr);
  manager.touch((void *)0x40, 1.f, nullptr);
  manager.touch((void *)0x50, 1.f, nullptr);

  {
    SCOPED_TRACE("insert all");
    CmdVector cmds;
    manager.update(&cookie, LogCmds { cmds });
    ASSERT_THAT(cmds, testing::ElementsAreArray(CmdVector {
      { &manager, TextureCmd::Insert, 0, 5 }
    }));
  }

  {
    SCOPED_TRACE("insert anywhere");
    manager.touch((void *)0x12, 1.f, nullptr);
    manager.touch((void *)0x15, 1.f, nullptr);
    manager.touch((void *)0x30, 2.f, nullptr);
    manager.touch((void *)0xff, 1.f, nullptr);

    CmdVector cmds;
    manager.update(&cookie, LogCmds { cmds });
    ASSERT_THAT(cmds, testing::ElementsAreArray(CmdVector {
      { &manager, TextureCmd::Insert, 1, 2 }, // 0x12 and 0x14
      { &manager, TextureCmd::Insert, 5, 1 }, // 0x35
      { &manager, TextureCmd::Insert, 8, 1 }, // 0xff
    }));
  }

  {
    SCOPED_TRACE("update");
    manager.invalidate((void *)0x12);
    manager.invalidate((void *)0x30);

    CmdVector cmds;
    manager.update(&cookie, LogCmds { cmds });
    ASSERT_THAT(cmds, testing::ElementsAreArray(CmdVector {
      { &manager, TextureCmd::Update, 1, 1 }, // 0x12
      { &manager, TextureCmd::Update, 4, 2 }, // 0x30 (scale=1 and 2)
    }));
  }

  {
    SCOPED_TRACE("remove anywhere");
    manager.remove((void *)0x12);
    manager.remove((void *)0x15);
    manager.remove((void *)0x30);

    CmdVector cmds;
    manager.update(&cookie, LogCmds { cmds });
    ASSERT_THAT(cmds, testing::ElementsAreArray(CmdVector {
      { &manager, TextureCmd::Remove, 1, 2 }, // 0x12 and 0x15
      { &manager, TextureCmd::Remove, 2, 2 }, // 0x30 (scale=1 and 2)
    }));
  }

  {
    SCOPED_TRACE("remove at the end");
    manager.remove((void *)0xff);

    CmdVector cmds;
    manager.update(&cookie, LogCmds { cmds });
    ASSERT_THAT(cmds, testing::ElementsAreArray(CmdVector {
      { &manager, TextureCmd::Remove, 4, 1 },
    }));
  }
}
