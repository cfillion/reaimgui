#include "api_helper.hpp"

#include "window.hpp"

#include <imgui/imgui.h>

DEFINE_API(void, FooBar, ((Window*, window)),
R"()",
{
  CHECK_WINDOW(window);
  window->enterFrame();
  ImGui::ShowDemoWindow();
});
