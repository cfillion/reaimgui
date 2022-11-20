/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "helper.hpp"

API_SECTION("Combo & List");

static std::vector<const char *> splitList(const char *buf, const int size)
{
  // REAPER's buf, buf_sz mechanism did not handle strings containing null
  // bytes (and len was inaccurate) prior to v6.44.
  if(size < 1 || buf[size - 1] != '\0')
    throw reascript_error { "requires REAPER v6.44 or newer (use BeginCombo or BeginListBox for wider compatibility)" };
  else if(size < 2 || buf[size - 2] != '\0')
    throw reascript_error { "items must be null-terminated" };

  std::vector<const char *> items;

  for(int i {}; i < size - 1; ++i) {
    items.push_back(buf);
    while(*buf++) ++i;
  }

  return items;
}

API_SUBSECTION("Combo Box (Dropdown)");

DEFINE_API(bool, BeginCombo, (ImGui_Context*,ctx)(const char*,label)
(const char*,preview_value)(int*,API_RO(flags)),
R"(The ImGui_BeginCombo/ImGui_EndCombo API allows you to manage your contents and selection state however you want it, by creating e.g. ImGui_Selectable items.

Default values: flags = ImGui_ComboFlags_None)",
{
  FRAME_GUARD;

  return ImGui::BeginCombo(label, preview_value,
    valueOr(API_RO(flags), ImGuiComboFlags_None));
});

DEFINE_API(void, EndCombo, (ImGui_Context*,ctx),
"Only call EndCombo() if ImGui_BeginCombo returns true!",
{
  FRAME_GUARD;
  ImGui::EndCombo();
});

DEFINE_API(bool, Combo, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(current_item))(const char*,items)(int,items_sz)
(int*,API_RO(popup_max_height_in_items)),
R"(Helper over ImGui_BeginCombo/ImGui_EndCombo for convenience purpose. Each item must be null-terminated (requires REAPER v6.44 or newer for EEL and Lua).

Default values: popup_max_height_in_items = -1)",
{
  FRAME_GUARD;

  const auto &strings { splitList(items, items_sz) };
  return ImGui::Combo(label, API_RW(current_item), strings.data(), strings.size(),
    valueOr(API_RO(popup_max_height_in_items), -1));
});

DEFINE_ENUM(ImGui, ComboFlags_None,           "Flags for ImGui_BeginCombo.");
DEFINE_ENUM(ImGui, ComboFlags_PopupAlignLeft, "Align the popup toward the left by default.");
DEFINE_ENUM(ImGui, ComboFlags_HeightSmall,    "Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use ImGui_SetNextWindowSizeConstraints prior to calling ImGui_BeginCombo.");
DEFINE_ENUM(ImGui, ComboFlags_HeightRegular,  "Max ~8 items visible (default).");
DEFINE_ENUM(ImGui, ComboFlags_HeightLarge,    "Max ~20 items visible.");
DEFINE_ENUM(ImGui, ComboFlags_HeightLargest,  "As many fitting items as possible.");
DEFINE_ENUM(ImGui, ComboFlags_NoArrowButton,  "Display on the preview box without the square arrow button.");
DEFINE_ENUM(ImGui, ComboFlags_NoPreview,      "Display only a square arrow button.");

API_SUBSECTION("List Boxes", "This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.");

DEFINE_API(bool, ListBox, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(current_item))(const char*,items)(int,items_sz)
(int*,API_RO(height_in_items)),
R"(This is an helper over ImGui_BeginListBox/ImGui_EndListBox for convenience purpose.

Each item must be null-terminated (requires REAPER v6.44 or newer for EEL and Lua).

Default values: height_in_items = -1)",
{
  FRAME_GUARD;

  const auto &strings { splitList(items, items_sz) };
  return ImGui::ListBox(label, API_RW(current_item), strings.data(), strings.size(),
    valueOr(API_RO(height_in_items), -1));
});

DEFINE_API(bool, BeginListBox, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RO(size_w))(double*,API_RO(size_h)),
R"(Open a framed scrolling region.  This is essentially a thin wrapper to using ImGui_BeginChild/ImGui_EndChild with some stylistic changes.

The ImGui_BeginListBox/ImGui_EndListBox API allows you to manage your contents and selection state however you want it, by creating e.g. ImGui_Selectable or any items.

- Choose frame width:   width  > 0.0: custom  /  width  < 0.0 or -FLT_MIN: right-align   /  width  = 0.0 (default): use current ItemWidth
- Choose frame height:  height > 0.0: custom  /  height < 0.0 or -FLT_MIN: bottom-align  /  height = 0.0 (default): arbitrary default height which can fit ~7 items

Default values: size_w = 0.0, size_h = 0.0

See ImGui_EndListBox.)",
{
  FRAME_GUARD;
  const ImVec2 size { valueOr(API_RO(size_w), 0.f),
                      valueOr(API_RO(size_h), 0.f) };
  return ImGui::BeginListBox(label, size);
});

DEFINE_API(void, EndListBox, (ImGui_Context*,ctx),
"Only call EndListBox() if ImGui_BeginListBox returned true!",
{
  FRAME_GUARD;
  ImGui::EndListBox();
});

API_SUBSECTION("Selectables",
R"(A selectable highlights when hovered, and can display another color when selected.
Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.)");

DEFINE_API(bool, Selectable, (ImGui_Context*,ctx)
(const char*,label)(bool*,API_RW(p_selected))
(int*,API_RO(flags))(double*,API_RO(size_w))(double*,API_RO(size_h)),
"Default values: flags = ImGui_SelectableFlags_None, size_w = 0.0, size_h = 0.0",
{
  FRAME_GUARD;
  bool selectedOmitted {};
  bool *selected { API_RW(p_selected) ? API_RW(p_selected) : &selectedOmitted };
  const ImGuiSelectableFlags flags
    { valueOr(API_RO(flags), ImGuiSelectableFlags_None) };
  const ImVec2 size { valueOr(API_RO(size_w), 0.f),
                      valueOr(API_RO(size_h), 0.f) };
  return ImGui::Selectable(label, selected, flags, size);
});

DEFINE_ENUM(ImGui, SelectableFlags_None,             "Flags for ImGui_Selectable.");
DEFINE_ENUM(ImGui, SelectableFlags_DontClosePopups,  "Clicking this doesn't close parent popup window.");
DEFINE_ENUM(ImGui, SelectableFlags_SpanAllColumns,   "Selectable frame can span all columns (text will still fit in current column).");
DEFINE_ENUM(ImGui, SelectableFlags_AllowDoubleClick, "Generate press events on double clicks too.");
DEFINE_ENUM(ImGui, SelectableFlags_Disabled,         "Cannot be selected, display grayed out text.");
DEFINE_ENUM(ImGui, SelectableFlags_AllowItemOverlap, "Hit testing to allow subsequent widgets to overlap this one.");
