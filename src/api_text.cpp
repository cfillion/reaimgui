#include "api_helper.hpp"

#include "colors.hpp"

DEFINE_API(void, Text, ((Window*,window))
((const char*,text)),
"",
{
  USE_WINDOW(window);
  ImGui::TextUnformatted(text);
});

DEFINE_API(void, TextColored, ((Window*,window))
((int,colorRGBA))((const char*,text)),
"Shortcut for PushStyleColor(ImGuiCol_Text, color); Text(text); PopStyleColor();",
{
  USE_WINDOW(window);

  ImVec4 color;
  Color::unpack(colorRGBA, color);
  ImGui::PushStyleColor(ImGuiCol_Text, color);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextDisabled, ((Window*,window))
((const char*,text)),
"",
{
  USE_WINDOW(window);
  const ImGuiStyle &style { ImGui::GetStyle() };
  ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextWrapped, ((Window*,window))
((const char*,text)),
"Shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().",
{
  USE_WINDOW(window);
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextUnformatted(text);
  ImGui::PopTextWrapPos();
});

DEFINE_API(void, LabelText, ((Window*,window))
((const char*,label))((const char*,text)),
"Display text+label aligned the same way as value+label widgets",
{
  USE_WINDOW(window);
  ImGui::LabelText(label, "%s", text);
});

DEFINE_API(void, BulletText, ((Window*,window))
((const char*,text)),
"Shortcut for Bullet()+Text()",
{
  USE_WINDOW(window);
  ImGui::Bullet();
  ImGui::TextUnformatted(text);
});

DEFINE_API(void, PushTextWrapPos, ((Window*,window))
((double*,wrapLocalPosXInOptional)),
R"(Push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space.

'wrapLocalPosX' defaults to 0.0.)",
{
  USE_WINDOW(window);
  ImGui::PushTextWrapPos(valueOr(wrapLocalPosXInOptional, 0.0));
});


DEFINE_API(void, PopTextWrapPos, ((Window*,window)),
"",
{
  USE_WINDOW(window);
  ImGui::PopTextWrapPos();
});
