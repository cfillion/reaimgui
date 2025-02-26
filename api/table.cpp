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

#include "../src/color.hpp"

API_SECTION("Table",
R"(See top of [imgui_tables.cpp](https://github.com/ocornut/imgui/blob/master/imgui_tables.cpp)
for general commentary.

See TableFlags* and TableColumnFlags* enums for a description of available flags.

The typical call flow is:
1. Call BeginTable.
2. Optionally call TableSetupColumn to submit column name/flags/defaults.
3. Optionally call TableSetupScrollFreeze to request scroll freezing of columns/rows.
4. Optionally call TableHeadersRow to submit a header row. Names are pulled from
   TableSetupColumn data.
5. Populate contents:
   - In most situations you can use TableNextRow + TableSetColumnIndex(N) to
     start appending into a column.
   - If you are using tables as a sort of grid, where every column is holding
     the same type of contents,
     you may prefer using TableNextColumn instead of
     TableNextRow + TableSetColumnIndex.
     TableNextColumn will automatically wrap-around into the next row if needed.
   - Summary of possible call flow:
     ```
     TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
     TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
                       TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
     TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
     ```
5. Call EndTable.
)");

API_FUNC(0_9, bool, BeginTable, (Context*,ctx)
(const char*,str_id) (int,columns) (RO<int*>,flags,ImGuiTableFlags_None)
(RO<double*>,outer_size_w,0.0) (RO<double*>,outer_size_h,0.0)
(RO<double*>,inner_width,0.0),
"")
{
  FRAME_GUARD;

  return ImGui::BeginTable(str_id, columns, API_GET(flags),
    ImVec2(API_GET(outer_size_w), API_GET(outer_size_h)),
    API_GET(inner_width));
}

API_FUNC(0_8, void, EndTable, (Context*,ctx),
"Only call EndTable() if BeginTable() returns true!")
{
  FRAME_GUARD;
  ImGui::EndTable();
}

API_FUNC(0_9, void, TableNextRow, (Context*,ctx)
(RO<int*>,row_flags,ImGuiTableRowFlags_None) (RO<double*>,min_row_height,0.0),
"Append into the first cell of a new row.")
{
  FRAME_GUARD;
  ImGui::TableNextRow(API_GET(row_flags), API_GET(min_row_height));
}

API_ENUM(0_1, ImGui, TableRowFlags_None, "For TableNextRow.");
API_ENUM(0_1, ImGui, TableRowFlags_Headers,
R"(Identify header row (set default background color + width of its contents
   accounted different for auto column width).)");

API_FUNC(0_8, bool, TableNextColumn, (Context*,ctx),
R"(Append into the next column (or first column of next row if currently in
last column). Return true when column is visible.)")
{
  FRAME_GUARD;
  return ImGui::TableNextColumn();
}

API_FUNC(0_8, bool, TableSetColumnIndex, (Context*,ctx)
(int,column_n),
"Append into the specified column. Return true when column is visible.")
{
  FRAME_GUARD;
  return ImGui::TableSetColumnIndex(column_n);
}

API_FUNC(0_1, int, TableGetColumnCount, (Context*,ctx),
"Return number of columns (value passed to BeginTable).")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnCount();
}

API_FUNC(0_1, int, TableGetColumnIndex, (Context*,ctx),
"Return current column index.")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnIndex();
}

API_FUNC(0_1, int, TableGetRowIndex, (Context*,ctx),
"Return current row index.")
{
  FRAME_GUARD;
  return ImGui::TableGetRowIndex();
}

API_SECTION_DEF(columns, ROOT_SECTION, "Header & Columns",
R"(Use TableSetupColumn() to specify label, resizing policy, default
width/weight, id, various other flags etc.

Use TableHeadersRow() to create a header row and automatically submit a
TableHeader() for each column. Headers are required to perform: reordering,
sorting, and opening the context menu. The context menu can also be made
available in columns body using TableFlags_ContextMenuInBody.

You may manually submit headers using TableNextRow() + TableHeader() calls, but
this is only useful in some advanced use cases (e.g. adding custom widgets in
header row).

Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when
scrolled.)");

API_FUNC(0_1, void, TableSetupColumn, (Context*,ctx)
(const char*,label) (RO<int*>,flags,ImGuiTableColumnFlags_None)
(RO<double*>,init_width_or_weight,0.0) (RO<int*>,user_id,0),
R"(Use to specify label, resizing policy, default width/weight, id,
various other flags etc.)")
{
  FRAME_GUARD;
  ImGui::TableSetupColumn(label, API_GET(flags),
    API_GET(init_width_or_weight), API_GET(user_id));
}

API_FUNC(0_1, void, TableSetupScrollFreeze, (Context*,ctx)
(int,cols) (int,rows),
"Lock columns/rows so they stay visible when scrolled.")
{
  FRAME_GUARD;
  ImGui::TableSetupScrollFreeze(cols, rows);
}

API_FUNC(0_1, void, TableHeader, (Context*,ctx)
(const char*,label),
"Submit one header cell manually (rarely used). See TableSetupColumn.")
{
  FRAME_GUARD;
  ImGui::TableHeader(label);
}

API_FUNC(0_1, void, TableHeadersRow, (Context*,ctx),
R"(Submit a row with headers cells based on data provided to TableSetupColumn
+ submit context menu.)")
{
  FRAME_GUARD;
  ImGui::TableHeadersRow();
}

API_FUNC(0_1, void, TableAngledHeadersRow, (Context*,ctx),
R"(Submit a row with angled headers for every column with the
TableColumnFlags_AngledHeader flag. Must be the first row.)")
{
  FRAME_GUARD;
  ImGui::TableAngledHeadersRow();
}

API_FUNC(0_1, const char*, TableGetColumnName, (Context*,ctx)
(RO<int*>,column_n,-1),
R"(Return "" if column didn't have a name declared by TableSetupColumn.
Pass -1 to use current column.)")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnName(API_GET(column_n));
}

API_FUNC(0_1, int, TableGetColumnFlags, (Context*,ctx)
(RO<int*>,column_n,-1),
R"(Return column flags so you can query their Enabled/Visible/Sorted/Hovered
status flags. Pass -1 to use current column.)")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnFlags(API_GET(column_n));
}

API_FUNC(0_4_1, void, TableSetColumnEnabled, (Context*,ctx)
(int,column_n) (bool,v),
R"(Change user-accessible enabled/disabled state of a column, set to false to
hide the column. Note that end-user can use the context menu to change this
themselves (right-click in headers, or right-click in columns body with
TableFlags_ContextMenuInBody).

- Require table to have the TableFlags_Hideable flag because we are manipulating
  user accessible state.
- Request will be applied during next layout, which happens on the first call to
  TableNextRow after Begin_Table.
- For the getter you can test
  (TableGetColumnFlags() & TableColumnFlags_IsEnabled) != 0.)")
{
  FRAME_GUARD;
  ImGui::TableSetColumnEnabled(column_n, v);
}

API_FUNC(0_9_2, int, TableGetHoveredColumn, (Context*,ctx),
R"(Returns hovered column or -1 when table is not hovered. Returns columns_count
if the unused space at the right of visible columns is hovered.

Can also use (TableGetColumnFlags() & TableColumnFlags_IsHovered) instead.)")
{
  FRAME_GUARD;
  return ImGui::TableGetHoveredColumn();
}

API_SECTION_DEF(columnFlags, columns, "Column Flags", "For TableSetupColumn.");
API_ENUM(0_1, ImGui, TableColumnFlags_None, "");
API_SECTION_P(columnFlags, "Input Configuration");
API_ENUM(0_5_5, ImGui, TableColumnFlags_Disabled,
R"(Overriding/master disable flag: hide column, won't show in context menu
   (unlike calling TableSetColumnEnabled which manipulates the user accessible state).)");
API_ENUM(0_1, ImGui, TableColumnFlags_DefaultHide,
  "Default as a hidden/disabled column.");
API_ENUM(0_1, ImGui, TableColumnFlags_DefaultSort, "Default as a sorting column.");
API_ENUM(0_1, ImGui, TableColumnFlags_WidthStretch,
R"(Column will stretch. Preferable with horizontal scrolling disabled
   (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).)");
API_ENUM(0_1, ImGui, TableColumnFlags_WidthFixed,
R"(Column will not stretch. Preferable with horizontal scrolling enabled
   (default if table sizing policy is _SizingFixedFit and table is resizable).)");
API_ENUM(0_1, ImGui, TableColumnFlags_NoResize, "Disable manual resizing.");
API_ENUM(0_1, ImGui, TableColumnFlags_NoReorder,
R"(Disable manual reordering this column, this will also prevent other columns
   from crossing over this column.)");
API_ENUM(0_1, ImGui, TableColumnFlags_NoHide,
  "Disable ability to hide/disable this column.");
API_ENUM(0_1, ImGui, TableColumnFlags_NoClip,
R"(Disable clipping for this column
   (all NoClip columns will render in a same draw command).)");
API_ENUM(0_1, ImGui, TableColumnFlags_NoSort,
R"(Disable ability to sort on this field
   (even if TableFlags_Sortable is set on the table).)");
API_ENUM(0_1, ImGui, TableColumnFlags_NoSortAscending,
  "Disable ability to sort in the ascending direction.");
API_ENUM(0_1, ImGui, TableColumnFlags_NoSortDescending,
  "Disable ability to sort in the descending direction.");
API_ENUM(0_5_5, ImGui, TableColumnFlags_NoHeaderLabel,
R"(TableHeadersRow will not submit horizontal label for this column.
   Convenient for some small columns. Name will still appear in context menu
   or in angled headers.)");
API_ENUM(0_1, ImGui, TableColumnFlags_NoHeaderWidth,
  "Disable header text width contribution to automatic column width.");
API_ENUM(0_1, ImGui, TableColumnFlags_PreferSortAscending,
  "Make the initial sort direction Ascending when first sorting on this column (default).");
API_ENUM(0_1, ImGui, TableColumnFlags_PreferSortDescending,
  "Make the initial sort direction Descending when first sorting on this column.");
API_ENUM(0_1, ImGui, TableColumnFlags_IndentEnable,
  "Use current Indent value when entering cell (default for column 0).");
API_ENUM(0_1, ImGui, TableColumnFlags_IndentDisable,
R"(Ignore current Indent value when entering cell (default for columns > 0).
   Indentation changes _within_ the cell will still be honored.)");
API_ENUM(0_9, ImGui, TableColumnFlags_AngledHeader,
R"(TableHeadersRow will submit an angled header row for this column.
   Note this will add an extra row.)");
API_SECTION_P(columnFlags, "Output Status", "Read-only via TableGetColumnFlags");
API_ENUM(0_1, ImGui, TableColumnFlags_IsEnabled,
R"(Status: is enabled == not hidden by user/api (referred to as "Hide" in
   _DefaultHide and _NoHide) flags.)");
API_ENUM(0_1, ImGui, TableColumnFlags_IsVisible,
  "Status: is visible == is enabled AND not clipped by scrolling.");
API_ENUM(0_1, ImGui, TableColumnFlags_IsSorted,
  "Status: is currently part of the sort specs.");
API_ENUM(0_1, ImGui, TableColumnFlags_IsHovered, "Status: is hovered by mouse.");

API_SUBSECTION("Sorting");

API_FUNC(0_1, bool, TableNeedSort, (Context*,ctx)
(W<bool*>,has_specs),
R"(Return true once when sorting specs have changed since last call,
or the first time. 'has_specs' is false when not sorting.

See TableGetColumnSortSpecs.)")
{
  FRAME_GUARD;
  if(ImGuiTableSortSpecs *specs {ImGui::TableGetSortSpecs()}) {
    if(has_specs) *has_specs = specs->SpecsCount > 0;

    const bool needSort {specs->SpecsDirty};
    specs->SpecsDirty = false;
    return needSort;
  }

  if(has_specs) *has_specs = false;
  return false;
}

API_FUNC(0_9, bool, TableGetColumnSortSpecs, (Context*,ctx) (int,id)
(W<int*>,column_index) (W<int*>,column_user_id) (W<int*>,sort_direction),
R"(Sorting specification for one column of a table.
Call while incrementing 'id' from 0 until false is returned.

- id:             Index of the sorting specification (always stored in order
  starting from 0, tables sorted on a single criteria will always have a 0 here)
- column_index:   Index of the column
- column_user_id: User ID of the column (if specified by a TableSetupColumn call)
- sort_direction: SortDirection_Ascending or SortDirection_Descending

See TableNeedSort.)")
{
  FRAME_GUARD;

  const ImGuiTableSortSpecs *specs {ImGui::TableGetSortSpecs()};
  if(!specs || id < 0 || id >= specs->SpecsCount)
    return false; // don't assert: user cannot know how many specs there are

  const ImGuiTableColumnSortSpecs &spec {specs->Specs[id]};
  if(column_index)   *column_index   = spec.ColumnIndex;
  if(column_user_id) *column_user_id = spec.ColumnUserID;
  if(sort_direction) *sort_direction = spec.SortDirection;

  return true;
}

API_ENUM(0_1, ImGui, SortDirection_None,       "");
API_ENUM(0_1, ImGui, SortDirection_Ascending,  "Ascending = 0->9, A->Z etc.");
API_ENUM(0_1, ImGui, SortDirection_Descending, "Descending = 9->0, Z->A etc.");

API_SUBSECTION("Background",
R"(Background colors are rendering in 3 layers:

- Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if set.
- Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if set.
- Layer 2: draw with CellBg color if set.

The purpose of the two row/columns layers is to let you decide if a background
color change should override or blend with the existing color.
When using TableFlags_RowBg on the table, each row has the RowBg0 color
automatically set for odd/even rows.
If you set the color of RowBg0 target, your color will override the existing
RowBg0 color.
If you set the color of RowBg1 or ColumnBg1 target, your color will blend over
the RowBg0 color.)");

API_FUNC(0_1, void, TableSetBgColor, (Context*,ctx)
(int,target) (int,color_rgba) (RO<int*>,column_n,-1),
R"(Change the color of a cell, row, or column.
See TableBgTarget_* flags for details.)")
{
  FRAME_GUARD;
  ImGui::TableSetBgColor(target,
    Color::fromBigEndian(color_rgba), API_GET(column_n));
}

API_ENUM(0_1, ImGui, TableBgTarget_None, "");
API_ENUM(0_1, ImGui, TableBgTarget_RowBg0,
R"(Set row background color 0 (generally used for background,
   automatically set when TableFlags_RowBg is used).)");
API_ENUM(0_1, ImGui, TableBgTarget_RowBg1,
  "Set row background color 1 (generally used for selection marking).");
API_ENUM(0_1, ImGui, TableBgTarget_CellBg,
  "Set cell background color (top-most color).");

API_SECTION_DEF(tableFlags, ROOT_SECTION, "Table Flags",
R"(For BeginTable.

- Important! Sizing policies have complex and subtle side effects,
  more so than you would expect. Read comments/demos carefully +
  experiment with live demos to get acquainted with them.
- The DEFAULT sizing policies are:
   - Default to TableFlags_SizingFixedFit
     if ScrollX is on, or if host window has WindowFlags_AlwaysAutoResize.
   - Default to TableFlags_SizingStretchSame if ScrollX is off.
- When ScrollX is off:
   - Table defaults to TableFlags_SizingStretchSame ->
     all Columns defaults to TableColumnFlags_WidthStretch with same weight.
   - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
   - Fixed Columns will generally obtain their requested width
     (unless the table cannot fit them all).
   - Stretch Columns will share the remaining width.
   - Mixed Fixed/Stretch columns is possible but has various side-effects on
     resizing behaviors.
     The typical use of mixing sizing policies is: any number of LEADING Fixed
     columns, followed by one or two TRAILING Stretch columns.
     (this is because the visible order of columns have subtle but necessary
     effects on how they react to manual resizing).
- When ScrollX is on:
   - Table defaults to TableFlags_SizingFixedFit ->
     all Columns defaults to TableColumnFlags_WidthFixed
   - Columns sizing policy allowed: Fixed/Auto mostly.
   - Fixed Columns can be enlarged as needed.
     Table will show a horizontal scrollbar if needed.
   - When using auto-resizing (non-resizable) fixed columns,
     querying the content width to use item right-alignment e.g.
     SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a feedback loop.
   - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on,
     UNLESS you have specified a value for 'inner_width' in BeginTable().
     If you specify a value for 'inner_width' then effectively the scrolling
     space is known and Stretch or mixed Fixed/Stretch columns become meaningful
     again.
- Read on documentation at the top of imgui_tables.cpp for details.)");

API_ENUM(0_1, ImGui, TableFlags_None, "");

API_SECTION_P(tableFlags, "Features");
API_ENUM(0_1, ImGui, TableFlags_Resizable, "Enable resizing columns.");
API_ENUM(0_1, ImGui, TableFlags_Reorderable,
R"(Enable reordering columns in header row
   (need calling TableSetupColumn + TableHeadersRow to display headers).)");
API_ENUM(0_1, ImGui, TableFlags_Hideable,
  "Enable hiding/disabling columns in context menu.");
API_ENUM(0_1, ImGui, TableFlags_Sortable,
R"(Enable sorting. Call TableNeedSort/TableGetColumnSortSpecs to obtain sort specs.
   Also see TableFlags_SortMulti and TableFlags_SortTristate.)");
API_ENUM(0_4, ImGui, TableFlags_NoSavedSettings,
  "Disable persisting columns order, width and sort settings in the .ini file.");
API_ENUM(0_1, ImGui, TableFlags_ContextMenuInBody,
R"(Right-click on columns body/contents will display table context menu.
   By default it is available in TableHeadersRow.)");

API_SECTION_P(tableFlags, "Decorations");
API_ENUM(0_1, ImGui, TableFlags_RowBg,
R"(Set each RowBg color with Col_TableRowBg or Col_TableRowBgAlt (equivalent of
   calling TableSetBgColor with TableBgTarget_RowBg0 on each row manually).)");
API_ENUM(0_1, ImGui, TableFlags_BordersInnerH, "Draw horizontal borders between rows.");
API_ENUM(0_1, ImGui, TableFlags_BordersOuterH, "Draw horizontal borders at the top and bottom.");
API_ENUM(0_1, ImGui, TableFlags_BordersInnerV, "Draw vertical borders between columns.");
API_ENUM(0_1, ImGui, TableFlags_BordersOuterV, "Draw vertical borders on the left and right sides.");
API_ENUM(0_1, ImGui, TableFlags_BordersH,      "Draw horizontal borders.");
API_ENUM(0_1, ImGui, TableFlags_BordersV,      "Draw vertical borders.");
API_ENUM(0_1, ImGui, TableFlags_BordersInner,  "Draw inner borders.");
API_ENUM(0_1, ImGui, TableFlags_BordersOuter,  "Draw outer borders.");
API_ENUM(0_1, ImGui, TableFlags_Borders,       "Draw all borders.");
// API_ENUM(ImGui, TableFlags_NoBordersInBody,
// R"([ALPHA] Disable vertical borders in columns Body
//    (borders will always appear in Headers). -> May move to style.)");
// API_ENUM(ImGui, TableFlags_NoBordersInBodyUntilResize,
// R"([ALPHA] Disable vertical borders in columns Body until hovered for resize
//    (borders will always appear in Headers). -> May move to style.)");

API_SECTION_P(tableFlags, "Sizing Policy", "(read above for defaults)");
API_ENUM(0_1, ImGui, TableFlags_SizingFixedFit,
R"(Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable),
   matching contents width.)");
API_ENUM(0_1, ImGui, TableFlags_SizingFixedSame,
R"(Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable),
   matching the maximum contents width of all columns.
   Implicitly enable TableFlags_NoKeepColumnsVisible.)");
API_ENUM(0_1, ImGui, TableFlags_SizingStretchProp,
R"(Columns default to _WidthStretch with default weights proportional to each
   columns contents widths.)");
API_ENUM(0_1, ImGui, TableFlags_SizingStretchSame,
R"(Columns default to _WidthStretch with default weights all equal,
   unless overriden by TableSetupColumn.)");

API_SECTION_P(tableFlags, "Sizing Extra Options");
API_ENUM(0_1, ImGui, TableFlags_NoHostExtendX,
R"(Make outer width auto-fit to columns, overriding outer_size.x value. Only
   available when ScrollX/ScrollY are disabled and Stretch columns are not used.)");
API_ENUM(0_1, ImGui, TableFlags_NoHostExtendY,
R"(Make outer height stop exactly at outer_size.y (prevent auto-extending table
   past the limit). Only available when ScrollX/ScrollY are disabled.
   Data below the limit will be clipped and not visible.)");
API_ENUM(0_1, ImGui, TableFlags_NoKeepColumnsVisible,
R"(Disable keeping column always minimally visible when ScrollX is off and table
   gets too small. Not recommended if columns are resizable.)");
API_ENUM(0_1, ImGui, TableFlags_PreciseWidths,
R"(Disable distributing remainder width to stretched columns (width allocation
   on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this
   flag: 33,33,33).
   With larger number of columns, resizing will appear to be less smooth.)");

API_SECTION_P(tableFlags, "Clipping");
API_ENUM(0_1, ImGui, TableFlags_NoClip,
R"(Disable clipping rectangle for every individual columns
   (reduce draw command count, items will be able to overflow into other columns).
   Generally incompatible with TableSetupScrollFreeze.)");

API_SECTION_P(tableFlags, "Padding");
API_ENUM(0_1, ImGui, TableFlags_PadOuterX,
R"(Default if TableFlags_BordersOuterV is on. Enable outermost padding.
   Generally desirable if you have headers.)");
API_ENUM(0_1, ImGui, TableFlags_NoPadOuterX,
  "Default if TableFlags_BordersOuterV is off. Disable outermost padding.");
API_ENUM(0_1, ImGui, TableFlags_NoPadInnerX,
R"(Disable inner padding between columns (double inner padding if
   TableFlags_BordersOuterV is on, single inner padding if BordersOuterV is off).)");

API_SECTION_P(tableFlags, "Scrolling");
API_ENUM(0_1, ImGui, TableFlags_ScrollX,
R"(Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable to
   specify the container size. Changes default sizing policy.
   Because this creates a child window, ScrollY is currently generally
   recommended when using ScrollX.)");
API_ENUM(0_1, ImGui, TableFlags_ScrollY,
R"(Enable vertical scrolling.
   Require 'outer_size' parameter of BeginTable to specify the container size.)");

API_SECTION_P(tableFlags, "Sorting");
API_ENUM(0_1, ImGui, TableFlags_SortMulti,
R"(Hold shift when clicking headers to sort on multiple column.
   TableGetColumnSortSpecs may return specs where (SpecsCount > 1).)");
API_ENUM(0_1, ImGui, TableFlags_SortTristate,
R"(Allow no sorting, disable default sorting.
   TableGetColumnSortSpecs may return specs where (SpecsCount == 0).)");

API_SECTION_P(tableFlags, "Miscellaneous");
API_ENUM(0_9, ImGui, TableFlags_HighlightHoveredColumn,
  "Highlight column headers when hovered (may evolve into a fuller highlight.)");
