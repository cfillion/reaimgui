#include "api_helper.hpp"

#include <vector>

// Allowing ReaScripts to input a null-separated string would be unsafe.
// REAPER's buf, buf_sz mechanism does not handle strings containing null
// bytes, so the user would have to specify the size manually. This would
// enable reading from arbitrary memory locations.
static bool makeNullSeparated(char *list)
{
  constexpr char ITEM_SEP { '\x1f' }; // ASCII Unit Separator
  const auto len { strlen(list) };

  if(len < 1 || list[len - 1] != ITEM_SEP || list[len] != '\0') {
    ReaScriptError("ReaImGui: items are not terminated with \\31 (unit separator)");
    return false;
  }

  for(char *p { list }; *p; ++p) {
    if(*p == ITEM_SEP)
      *p = '\0';
  }

  return true;
}

#define NULL_SEPARATED_LIST(list, ...) \
  if(!makeNullSeparated(list)) return __VA_ARGS__;

static std::vector<const char *> nullSeparatedToVector(const char *list)
{
  std::vector<const char *> strings;
  while(*list) {
    strings.push_back(list);
    list += strlen(list) + 1;
  }
  return strings;
}

// Widgets: Combo Box
DEFINE_API(bool, BeginCombo, ((ImGui_Context*,ctx))((const char*,label))
((const char*,previewValue))((int*,API_RO(flags))),
R"(The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.

Default values: flags = ImGui_ComboFlags_None)",
{
  ENTER_CONTEXT(ctx, false);

  return ImGui::BeginCombo(label, previewValue,
    valueOr(API_RO(flags), ImGuiComboFlags_None));
});

DEFINE_API(void, EndCombo, ((ImGui_Context*,ctx)),
"Only call EndCombo() if BeginCombo() returns true!",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndCombo();
});

DEFINE_API(bool, Combo, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(currentItem)))((char*,items))
((int*,API_RO(popupMaxHeightInItems))),
R"(Helper over BeginCombo()/EndCombo() for convenience purpose. Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

Default values: popupMaxHeightInItems = -1)",
{
  ENTER_CONTEXT(ctx, false);
  NULL_SEPARATED_LIST(items, false);

  return ImGui::Combo(label, API_RW(currentItem), items,
    valueOr(API_RO(popupMaxHeightInItems), -1));
});

// Widgets: List Boxes
DEFINE_API(bool, ListBox, ((ImGui_Context*,ctx))((const char*,label))
((int*,API_RW(currentItem)))((char*,items))((int*,API_RO(heightInItems))),
R"(This is an helper over BeginListBox()/EndListBox() for convenience purpose. This is analoguous to how Combos are created.

Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

'heightInItems' defaults to -1.)",
{
  ENTER_CONTEXT(ctx, false);
  NULL_SEPARATED_LIST(items, false);

  const auto &strings { nullSeparatedToVector(items) };
  return ImGui::ListBox(label, API_RW(currentItem), strings.data(), strings.size(),
    valueOr(API_RO(heightInItems), -1));
});

DEFINE_API(bool, BeginListBox, ((ImGui_Context*,ctx))
((const char*,label))((double*,API_RO(width)))((double*,API_RO(height))),
R"(Open a framed scrolling region.  This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.

The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() or any items.

- Choose frame width:   width  > 0.0: custom  /  width  < 0.0 or -FLT_MIN: right-align   /  width  = 0.0 (default): use current ItemWidth
- Choose frame height:  height > 0.0: custom  /  height < 0.0 or -FLT_MIN: bottom-align  /  height = 0.0 (default): arbitrary default height which can fit ~7 items

Default values: width = 0, height = 0

See ImGui_EndListBox.)",
{
  ENTER_CONTEXT(ctx, false);

  return ImGui::BeginListBox(label,
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)));
});

DEFINE_API(void, EndListBox, ((ImGui_Context*,ctx)),
R"(Only call EndListBox() if BeginListBox() returned true!

See ImGui_BeginListBox.)",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndListBox();
});
