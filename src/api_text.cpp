#include "api_helper.hpp"

DEFINE_API(void, Text, ((Window*, window))((const char*, text)),
"",
{
  USE_WINDOW(window);
  ImGui::TextUnformatted(text);
});

// TextColored

DEFINE_API(void, TextDisabled, ((Window*, window))((const char*, text)),
"",
{
  USE_WINDOW(window);

  const ImGuiStyle &style { ImGui::GetStyle() };
  ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

// TextWrapped

DEFINE_API(void, LabelText, ((Window*,window))
((const char*,label))((const char*,text)),
"Display text+label aligned the same way as value+label widgets",
{
  USE_WINDOW(window);
  ImGui::LabelText(label, "%s", text);
});

DEFINE_API(void, BulletText, ((Window*, window))((const char*, text)),
"Shortcut for Bullet()+Text()",
{
  USE_WINDOW(window);
  ImGui::Bullet();
  ImGui::TextUnformatted(text);
});
