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
((const char*,previewValue))((int*,flagsInOptional)),
R"(The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.

Default values: flags = ImGui_ComboFlags_None)",
{
  ENTER_CONTEXT(ctx, false);

  return ImGui::BeginCombo(label, previewValue,
    valueOr(flagsInOptional, ImGuiComboFlags_None));
});

DEFINE_API(void, EndCombo, ((ImGui_Context*,ctx)),
"Only call EndCombo() if BeginCombo() returns true!",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndCombo();
});

DEFINE_API(bool, Combo, ((ImGui_Context*,ctx))
((const char*,label))((int*,currentItemInOut))((char*,items))
((int*,popupMaxHeightInItemsInOptional)),
R"(Helper over BeginCombo()/EndCombo() for convenience purpose. Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

'popupMaxHeightInItems' defaults to -1.)",
{
  ENTER_CONTEXT(ctx, false);
  NULL_SEPARATED_LIST(items, false);

  return ImGui::Combo(label, currentItemInOut, items,
    valueOr(popupMaxHeightInItemsInOptional, -1));
});

// Widgets: List Boxes
DEFINE_API(bool, ListBox, ((ImGui_Context*,ctx))((const char*,label))
((int*,currentItemInOut))((char*,items))((int*,heightInItemsInOptional)),
R"(Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

'heightInItems' defaults to -1.)",
{
  ENTER_CONTEXT(ctx, false);
  NULL_SEPARATED_LIST(items, false);

  const auto &strings { nullSeparatedToVector(items) };
  return ImGui::ListBox(label, currentItemInOut, strings.data(), strings.size(),
    valueOr(heightInItemsInOptional, -1));
});
// IMGUI_API bool          ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
// IMGUI_API bool          ListBox(const char* label, int* current_item, bool (*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int height_in_items = -1);
// - FIXME: To be consistent with all the newer API, ListBoxHeader/ListBoxFooter should in reality be called BeginListBox/EndListBox. Will rename them.
// IMGUI_API bool          ListBoxHeader(const char* label, const ImVec2& size = ImVec2(0, 0)); // use if you want to reimplement ListBox() will custom data or interactions. if the function return true, you can output elements then call ListBoxFooter() afterwards.
// IMGUI_API bool          ListBoxHeader(const char* label, int items_count, int height_in_items = -1); // "
// IMGUI_API void          ListBoxFooter();                                                    // terminate the scrolling region. only call ListBoxFooter() if ListBoxHeader() returned true!
