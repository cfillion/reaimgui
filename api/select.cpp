/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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
  assertValid(buf);

  // REAPER's buf, buf_sz mechanism did not handle strings containing null
  // bytes (and len was inaccurate) prior to v6.44.
  if(size < 1 || buf[size - 1] != '\0') {
    throw reascript_error {"requires REAPER v6.44 or newer"
      " (use BeginCombo or BeginListBox for wider compatibility)"};
  }
  else if(size < 2 || buf[size - 2] != '\0')
    throw reascript_error {"items must be null-terminated"};

  std::vector<const char *> items;

  for(int i {}; i < size - 1; ++i) {
    items.push_back(buf);
    while(*buf++) ++i;
  }

  return items;
}

API_SUBSECTION("Combo Box (Dropdown)");

API_FUNC(0_1, bool, BeginCombo, (Context*,ctx) (const char*,label)
(const char*,preview_value) (RO<int*>,flags,ImGuiComboFlags_None),
R"(The BeginCombo/EndCombo API allows you to manage your contents and selection
state however you want it, by creating e.g. Selectable items.)")
{
  FRAME_GUARD;

  return ImGui::BeginCombo(label, preview_value, API_GET(flags));
}

API_FUNC(0_1, void, EndCombo, (Context*,ctx),
"Only call EndCombo() if BeginCombo returns true!")
{
  FRAME_GUARD;
  ImGui::EndCombo();
}

API_FUNC(0_7, bool, Combo, (Context*,ctx)
(const char*,label) (RW<int*>,current_item) (const char*,items) (int,items_sz)
(RO<int*>,popup_max_height_in_items,-1),
R"(Helper over BeginCombo/EndCombo for convenience purpose. Each item must be
null-terminated (requires REAPER v6.44 or newer for EEL and Lua).)")
{
  FRAME_GUARD;
  assertValid(current_item);

  const auto &strings {splitList(items, items_sz)};
  return ImGui::Combo(label, current_item,
    strings.data(), strings.size(), API_GET(popup_max_height_in_items));
}

API_ENUM(0_1, ImGui, ComboFlags_None, "");
API_ENUM(0_1, ImGui, ComboFlags_PopupAlignLeft,
  "Align the popup toward the left by default.");
API_ENUM(0_1, ImGui, ComboFlags_HeightSmall,
R"(Max ~4 items visible. Tip: If you want your combo popup to be a specific size
you can use SetNextWindowSizeConstraints prior to calling BeginCombo.)");
API_ENUM(0_1, ImGui, ComboFlags_HeightRegular,  "Max ~8 items visible (default).");
API_ENUM(0_1, ImGui, ComboFlags_HeightLarge,    "Max ~20 items visible.");
API_ENUM(0_1, ImGui, ComboFlags_HeightLargest,  "As many fitting items as possible.");
API_ENUM(0_1, ImGui, ComboFlags_NoArrowButton,
  "Display on the preview box without the square arrow button.");
API_ENUM(0_1, ImGui, ComboFlags_NoPreview,      "Display only a square arrow button.");
API_ENUM(0_9, ImGui, ComboFlags_WidthFitPreview,
  "Width dynamically calculated from preview contents.");

API_SUBSECTION("List Boxes",
R"(This is essentially a thin wrapper to using BeginChild/EndChild with the
ChildFlags_FrameStyle flag for stylistic changes + displaying a label.)");

API_FUNC(0_7, bool, ListBox, (Context*,ctx) (const char*,label)
(RW<int*>,current_item) (const char*,items) (int,items_sz)
(RO<int*>,height_in_items,-1),
R"(This is an helper over BeginListBox/EndListBox for convenience purpose.

Each item must be null-terminated (requires REAPER v6.44 or newer for EEL and Lua).)")
{
  FRAME_GUARD;
  assertValid(current_item);

  const auto &strings {splitList(items, items_sz)};
  return ImGui::ListBox(label, current_item,
    strings.data(), strings.size(), API_GET(height_in_items));
}

API_FUNC(0_1, bool, BeginListBox, (Context*,ctx)
(const char*,label) (RO<double*>,size_w,0.0) (RO<double*>,size_h,0.0),
R"(Open a framed scrolling region.

You can submit contents and manage your selection state however you want it,
by creating e.g. Selectable or any other items.

- Choose frame width:
  - width  > 0.0: custom
  - width  < 0.0 or -FLT_MIN: right-align
  - width  = 0.0 (default): use current ItemWidth
- Choose frame height:
  - height > 0.0: custom
  - height < 0.0 or -FLT_MIN: bottom-align
  - height = 0.0 (default): arbitrary default height which can fit ~7 items

See EndListBox.)")
{
  FRAME_GUARD;
  return ImGui::BeginListBox(label, ImVec2(API_GET(size_w), API_GET(size_h)));
}

API_FUNC(0_1, void, EndListBox, (Context*,ctx),
"Only call EndListBox() if BeginListBox returned true!")
{
  FRAME_GUARD;
  ImGui::EndListBox();
}

API_SUBSECTION("Selectables",
R"(A selectable highlights when hovered, and can display another color when
selected. Neighbors selectable extend their highlight bounds in order to leave
no gap between them. This is so a series of selected Selectable appear
contiguous.)");

API_FUNC(0_1, bool, Selectable, (Context*,ctx)
(const char*,label) (RWO<bool*>,p_selected)
(RO<int*>,flags,ImGuiSelectableFlags_None)
(RO<double*>,size_w,0.0) (RO<double*>,size_h,0.0),
"")
{
  FRAME_GUARD;
  bool dummy_selected {};
  if(!p_selected)
    p_selected = &dummy_selected;
  return ImGui::Selectable(label, p_selected, API_GET(flags),
    ImVec2(API_GET(size_w), API_GET(size_h)));
}

API_ENUM(0_1, ImGui, SelectableFlags_None, "");
API_ENUM(0_1, ImGui, SelectableFlags_DontClosePopups,
  "Clicking this doesn't close parent popup window.");
API_ENUM(0_1, ImGui, SelectableFlags_SpanAllColumns,
  "Frame will span all columns of its container table (text will still fit in current column).");
API_ENUM(0_1, ImGui, SelectableFlags_AllowDoubleClick,
  "Generate press events on double clicks too.");
API_ENUM(0_1, ImGui, SelectableFlags_Disabled,
  "Cannot be selected, display grayed out text.");
API_ENUM(0_9, ImGui, SelectableFlags_AllowOverlap,
  "Hit testing to allow subsequent widgets to overlap this one.");
