#include "api_helper.hpp"

DEFINE_API(void, Text, ((ImGui_Context*,ctx))
((const char*,text)),
"",
{
  ENTER_CONTEXT(ctx);
  ImGui::TextUnformatted(text);
});

DEFINE_API(void, TextColored, ((ImGui_Context*,ctx))
((int,colorRGBA))((const char*,text)),
"Shortcut for PushStyleColor(ImGuiCol_Text, color); Text(text); PopStyleColor();",
{
  ENTER_CONTEXT(ctx);

  ImVec4 color { Color(colorRGBA) };
  ImGui::PushStyleColor(ImGuiCol_Text, color);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextDisabled, ((ImGui_Context*,ctx))
((const char*,text)),
"",
{
  ENTER_CONTEXT(ctx);
  const ImGuiStyle &style { ImGui::GetStyle() };
  ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextWrapped, ((ImGui_Context*,ctx))
((const char*,text)),
"Shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().",
{
  ENTER_CONTEXT(ctx);
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextUnformatted(text);
  ImGui::PopTextWrapPos();
});

DEFINE_API(void, LabelText, ((ImGui_Context*,ctx))
((const char*,label))((const char*,text)),
"Display text+label aligned the same way as value+label widgets",
{
  ENTER_CONTEXT(ctx);
  ImGui::LabelText(label, "%s", text);
});

DEFINE_API(void, BulletText, ((ImGui_Context*,ctx))
((const char*,text)),
"Shortcut for Bullet()+Text()",
{
  ENTER_CONTEXT(ctx);
  ImGui::Bullet();
  ImGui::TextUnformatted(text);
});

DEFINE_API(void, PushTextWrapPos, ((ImGui_Context*,ctx))
((double*,wrapLocalPosXInOptional)),
R"(Push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space.

Default values: wrapLocalPosX = 0.0)",
{
  ENTER_CONTEXT(ctx);
  ImGui::PushTextWrapPos(valueOr(wrapLocalPosXInOptional, 0.0));
});


DEFINE_API(void, PopTextWrapPos, ((ImGui_Context*,ctx)),
"",
{
  ENTER_CONTEXT(ctx);
  ImGui::PopTextWrapPos();
});
