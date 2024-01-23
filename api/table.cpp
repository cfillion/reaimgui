/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

DEFINE_API(bool, BeginTable, (ImGui_Context*,ctx)
(const char*,str_id)(int,column)(int*,API_RO(flags),ImGuiTableFlags_None)
(double*,API_RO(outer_size_w),0.0)(double*,API_RO(outer_size_h),0.0)
(double*,API_RO(inner_width),0.0),
"")
{
  FRAME_GUARD;

  return ImGui::BeginTable(str_id, column, API_RO_GET(flags),
    ImVec2(API_RO_GET(outer_size_w), API_RO_GET(outer_size_h)),
    API_RO_GET(inner_width));
}

DEFINE_API(void, EndTable, (ImGui_Context*,ctx),
"Only call EndTable() if BeginTable() returns true!")
{
  FRAME_GUARD;
  ImGui::EndTable();
}

DEFINE_API(void, TableNextRow, (ImGui_Context*,ctx)
(int*,API_RO(row_flags),ImGuiTableRowFlags_None)
(double*,API_RO(min_row_height),0.0),
"Append into the first cell of a new row.")
{
  FRAME_GUARD;
  ImGui::TableNextRow(API_RO_GET(row_flags), API_RO_GET(min_row_height));
}

DEFINE_ENUM(ImGui, TableRowFlags_None, "For TableNextRow.");
DEFINE_ENUM(ImGui, TableRowFlags_Headers,
R"(Identify header row (set default background color + width of its contents
   accounted different for auto column width).)");

DEFINE_API(bool, TableNextColumn, (ImGui_Context*,ctx),
R"(Append into the next column (or first column of next row if currently in
last column). Return true when column is visible.)")
{
  FRAME_GUARD;
  return ImGui::TableNextColumn();
}

DEFINE_API(bool, TableSetColumnIndex, (ImGui_Context*,ctx)
(int,column_n),
"Append into the specified column. Return true when column is visible.")
{
  FRAME_GUARD;
  return ImGui::TableSetColumnIndex(column_n);
}

DEFINE_API(int, TableGetColumnCount, (ImGui_Context*,ctx),
"Return number of columns (value passed to BeginTable).")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnCount();
}

DEFINE_API(int, TableGetColumnIndex, (ImGui_Context*,ctx),
"Return current column index.")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnIndex();
}

DEFINE_API(int, TableGetRowIndex, (ImGui_Context*,ctx),
"Return current row index.")
{
  FRAME_GUARD;
  return ImGui::TableGetRowIndex();
}

DEFINE_SECTION(columns, ROOT_SECTION, "Header & Columns",
R"(Use TableSetupColumn() to specify label, resizing policy, default
width/weight, id, various other flags etc.

Use TableHeadersRow() to create a header row and automatically submit a
TableHeader() for each column. Headers are required to perform: reordering,
sorting, and opening the context menu. The context menu can also be made
available in columns body using ImGuiTableFlags_ContextMenuInBody.

You may manually submit headers using TableNextRow() + TableHeader() calls, but
this is only useful in some advanced use cases (e.g. adding custom widgets in
header row).

Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when
scrolled.)");

DEFINE_API(void, TableSetupColumn, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RO(flags),ImGuiTableColumnFlags_None)
(double*,API_RO(init_width_or_weight),0.0)
(int*,API_RO(user_id),0),
R"(Use to specify label, resizing policy, default width/weight, id,
various other flags etc.)")
{
  FRAME_GUARD;
  ImGui::TableSetupColumn(label, API_RO_GET(flags),
    API_RO_GET(init_width_or_weight), API_RO_GET(user_id));
}

DEFINE_API(void, TableSetupScrollFreeze, (ImGui_Context*,ctx)
(int,cols)(int,rows),
"Lock columns/rows so they stay visible when scrolled.")
{
  FRAME_GUARD;
  ImGui::TableSetupScrollFreeze(cols, rows);
}

DEFINE_API(void, TableHeadersRow, (ImGui_Context*,ctx),
R"(Submit all headers cells based on data provided to TableSetupColumn +
submit context menu.)")
{
  FRAME_GUARD;
  ImGui::TableHeadersRow();
}

DEFINE_API(void, TableHeader, (ImGui_Context*,ctx)
(const char*,label),
"Submit one header cell manually (rarely used). See TableSetupColumn.")
{
  FRAME_GUARD;
  ImGui::TableHeader(label);
}

DEFINE_API(const char*, TableGetColumnName, (ImGui_Context*,ctx)
(int*,API_RO(column_n),-1),
R"(Return "" if column didn't have a name declared by TableSetupColumn.
Pass -1 to use current column.)")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnName(API_RO_GET(column_n));
}

DEFINE_API(int, TableGetColumnFlags, (ImGui_Context*,ctx)
(int*,API_RO(column_n),-1),
R"(Return column flags so you can query their Enabled/Visible/Sorted/Hovered
status flags. Pass -1 to use current column.)")
{
  FRAME_GUARD;
  return ImGui::TableGetColumnFlags(API_RO_GET(column_n));
}

DEFINE_API(void, TableSetColumnEnabled, (ImGui_Context*,ctx)
(int,column_n)(bool,v),
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

DEFINE_SECTION(columnFlags, columns, "Column Flags", "For TableSetupColumn.");
DEFINE_ENUM(ImGui, TableColumnFlags_None, "");
API_SECTION_P(columnFlags, "Input configuration");
DEFINE_ENUM(ImGui, TableColumnFlags_Disabled,
R"(Overriding/master disable flag: hide column, won't show in context menu
   (unlike calling TableSetColumnEnabled which manipulates the user accessible state).)");
DEFINE_ENUM(ImGui, TableColumnFlags_DefaultHide,
  "Default as a hidden/disabled column.");
DEFINE_ENUM(ImGui, TableColumnFlags_DefaultSort, "Default as a sorting column.");
DEFINE_ENUM(ImGui, TableColumnFlags_WidthStretch,
R"(Column will stretch. Preferable with horizontal scrolling disabled
   (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).)");
DEFINE_ENUM(ImGui, TableColumnFlags_WidthFixed,
R"(Column will not stretch. Preferable with horizontal scrolling enabled
   (default if table sizing policy is _SizingFixedFit and table is resizable).)");
DEFINE_ENUM(ImGui, TableColumnFlags_NoResize, "Disable manual resizing.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoReorder,
R"(Disable manual reordering this column, this will also prevent other columns
   from crossing over this column.)");
DEFINE_ENUM(ImGui, TableColumnFlags_NoHide,
  "Disable ability to hide/disable this column.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoClip,
R"(Disable clipping for this column
   (all NoClip columns will render in a same draw command).)");
DEFINE_ENUM(ImGui, TableColumnFlags_NoSort,
R"(Disable ability to sort on this field
   (even if TableFlags_Sortable is set on the table).)");
DEFINE_ENUM(ImGui, TableColumnFlags_NoSortAscending,
  "Disable ability to sort in the ascending direction.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoSortDescending,
  "Disable ability to sort in the descending direction.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoHeaderLabel,
R"(TableHeadersRow will not submit label for this column.
   Convenient for some small columns. Name will still appear in context menu.)");
DEFINE_ENUM(ImGui, TableColumnFlags_NoHeaderWidth,
  "Disable header text width contribution to automatic column width.");
DEFINE_ENUM(ImGui, TableColumnFlags_PreferSortAscending,
  "Make the initial sort direction Ascending when first sorting on this column (default).");
DEFINE_ENUM(ImGui, TableColumnFlags_PreferSortDescending,
  "Make the initial sort direction Descending when first sorting on this column.");
DEFINE_ENUM(ImGui, TableColumnFlags_IndentEnable,
  "Use current Indent value when entering cell (default for column 0).");
DEFINE_ENUM(ImGui, TableColumnFlags_IndentDisable,
R"(Ignore current Indent value when entering cell (default for columns > 0).
   Indentation changes _within_ the cell will still be honored.)");
API_SECTION_P(columnFlags, "Output status", "Read-only via TableGetColumnFlags");
DEFINE_ENUM(ImGui, TableColumnFlags_IsEnabled,
R"(Status: is enabled == not hidden by user/api (referred to as "Hide" in
   _DefaultHide and _NoHide) flags.)");
DEFINE_ENUM(ImGui, TableColumnFlags_IsVisible,
  "Status: is visible == is enabled AND not clipped by scrolling.");
DEFINE_ENUM(ImGui, TableColumnFlags_IsSorted,
  "Status: is currently part of the sort specs.");
DEFINE_ENUM(ImGui, TableColumnFlags_IsHovered, "Status: is hovered by mouse.");


API_SUBSECTION("Sorting");

DEFINE_API(bool, TableNeedSort, (ImGui_Context*,ctx)
(bool*,API_W(has_specs)),
R"(Return true once when sorting specs have changed since last call,
or the first time. 'has_specs' is false when not sorting.

See TableGetColumnSortSpecs.)")
{
  FRAME_GUARD;
  if(ImGuiTableSortSpecs *specs { ImGui::TableGetSortSpecs() }) {
    if(API_W(has_specs)) *API_W(has_specs) = true;

    const bool needSort { specs->SpecsDirty };
    specs->SpecsDirty = false;
    return needSort;
  }

  if(API_W(has_specs)) *API_W(has_specs) = false;
  return false;
}

DEFINE_API(bool, TableGetColumnSortSpecs, (ImGui_Context*,ctx)
(int,id)
(int*,API_W(column_user_id))(int*,API_W(column_index))
(int*,API_W(sort_order))(int*,API_W(sort_direction)),
R"(Sorting specification for one column of a table.
Call while incrementing 'id' from 0 until false is returned.

- ColumnUserID:  User id of the column (if specified by a TableSetupColumn call)
- ColumnIndex:   Index of the column
- SortOrder:     Index within parent SortSpecs (always stored in order starting
  from 0, tables sorted on a single criteria will always have a 0 here)
- SortDirection: SortDirection_Ascending or SortDirection_Descending
  (you can use this or SortSign, whichever is more convenient for your sort
  function)

See TableNeedSort.)")
{
  FRAME_GUARD;

  const ImGuiTableSortSpecs *specs { ImGui::TableGetSortSpecs() };
  if(!specs || id < 0 || id >= specs->SpecsCount)
    return false; // don't assert: user cannot know how many specs there are

  const ImGuiTableColumnSortSpecs &spec { specs->Specs[id] };
  if(API_W(column_user_id)) *API_W(column_user_id) = spec.ColumnUserID;
  if(API_W(column_index))   *API_W(column_index)   = spec.ColumnIndex;
  if(API_W(sort_order))     *API_W(sort_order)     = spec.SortOrder;
  if(API_W(sort_direction)) *API_W(sort_direction) = spec.SortDirection;
  return true;
}

DEFINE_ENUM(ImGui, SortDirection_None,       "");
DEFINE_ENUM(ImGui, SortDirection_Ascending,  "Ascending = 0->9, A->Z etc.");
DEFINE_ENUM(ImGui, SortDirection_Descending, "Descending = 9->0, Z->A etc.");

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

DEFINE_API(void, TableSetBgColor, (ImGui_Context*,ctx)
(int,target)(int,color_rgba)(int*,API_RO(column_n),-1),
R"(Change the color of a cell, row, or column.
See TableBgTarget_* flags for details.)")
{
  FRAME_GUARD;
  ImGui::TableSetBgColor(target,
    Color::fromBigEndian(color_rgba), API_RO_GET(column_n));
}

DEFINE_ENUM(ImGui, TableBgTarget_None, "");
DEFINE_ENUM(ImGui, TableBgTarget_RowBg0,
R"(Set row background color 0 (generally used for background,
   automatically set when TableFlags_RowBg is used).)");
DEFINE_ENUM(ImGui, TableBgTarget_RowBg1,
  "Set row background color 1 (generally used for selection marking).");
DEFINE_ENUM(ImGui, TableBgTarget_CellBg,
  "Set cell background color (top-most color).");

DEFINE_SECTION(tableFlags, ROOT_SECTION, "Table Flags",
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

DEFINE_ENUM(ImGui, TableFlags_None, "");

API_SECTION_P(tableFlags, "Features");
DEFINE_ENUM(ImGui, TableFlags_Resizable, "Enable resizing columns.");
DEFINE_ENUM(ImGui, TableFlags_Reorderable,
R"(Enable reordering columns in header row
   (need calling TableSetupColumn + TableHeadersRow to display headers).)");
DEFINE_ENUM(ImGui, TableFlags_Hideable,
  "Enable hiding/disabling columns in context menu.");
DEFINE_ENUM(ImGui, TableFlags_Sortable,
R"(Enable sorting. Call TableNeedSort/TableGetColumnSortSpecs to obtain sort specs.
   Also see TableFlags_SortMulti and TableFlags_SortTristate.)");
DEFINE_ENUM(ImGui, TableFlags_NoSavedSettings,
  "Disable persisting columns order, width and sort settings in the .ini file.");
DEFINE_ENUM(ImGui, TableFlags_ContextMenuInBody,
R"(Right-click on columns body/contents will display table context menu.
   By default it is available in TableHeadersRow.)");

API_SECTION_P(tableFlags, "Decorations");
DEFINE_ENUM(ImGui, TableFlags_RowBg,
R"(Set each RowBg color with Col_TableRowBg or Col_TableRowBgAlt (equivalent of
   calling TableSetBgColor with TableBgTarget_RowBg0 on each row manually).)");
DEFINE_ENUM(ImGui, TableFlags_BordersInnerH, "Draw horizontal borders between rows.");
DEFINE_ENUM(ImGui, TableFlags_BordersOuterH, "Draw horizontal borders at the top and bottom.");
DEFINE_ENUM(ImGui, TableFlags_BordersInnerV, "Draw vertical borders between columns.");
DEFINE_ENUM(ImGui, TableFlags_BordersOuterV, "Draw vertical borders on the left and right sides.");
DEFINE_ENUM(ImGui, TableFlags_BordersH,      "Draw horizontal borders.");
DEFINE_ENUM(ImGui, TableFlags_BordersV,      "Draw vertical borders.");
DEFINE_ENUM(ImGui, TableFlags_BordersInner,  "Draw inner borders.");
DEFINE_ENUM(ImGui, TableFlags_BordersOuter,  "Draw outer borders.");
DEFINE_ENUM(ImGui, TableFlags_Borders,       "Draw all borders.");
// DEFINE_ENUM(ImGui, TableFlags_NoBordersInBody,
// R"([ALPHA] Disable vertical borders in columns Body
//    (borders will always appear in Headers). -> May move to style.)");
// DEFINE_ENUM(ImGui, TableFlags_NoBordersInBodyUntilResize,
// R"([ALPHA] Disable vertical borders in columns Body until hovered for resize
//    (borders will always appear in Headers). -> May move to style.)");

API_SECTION_P(tableFlags, "Sizing Policy", "(read above for defaults)");
DEFINE_ENUM(ImGui, TableFlags_SizingFixedFit,
R"(Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable),
   matching contents width.)");
DEFINE_ENUM(ImGui, TableFlags_SizingFixedSame,
R"(Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable),
   matching the maximum contents width of all columns.
   Implicitly enable TableFlags_NoKeepColumnsVisible.)");
DEFINE_ENUM(ImGui, TableFlags_SizingStretchProp,
R"(Columns default to _WidthStretch with default weights proportional to each
   columns contents widths.)");
DEFINE_ENUM(ImGui, TableFlags_SizingStretchSame,
R"(Columns default to _WidthStretch with default weights all equal,
   unless overriden by TableSetupColumn.)");

API_SECTION_P(tableFlags, "Sizing Extra Options");
DEFINE_ENUM(ImGui, TableFlags_NoHostExtendX,
R"(Make outer width auto-fit to columns, overriding outer_size.x value. Only
   available when ScrollX/ScrollY are disabled and Stretch columns are not used.)");
DEFINE_ENUM(ImGui, TableFlags_NoHostExtendY,
R"(Make outer height stop exactly at outer_size.y (prevent auto-extending table
   past the limit). Only available when ScrollX/ScrollY are disabled.
   Data below the limit will be clipped and not visible.)");
DEFINE_ENUM(ImGui, TableFlags_NoKeepColumnsVisible,
R"(Disable keeping column always minimally visible when ScrollX is off and table
   gets too small. Not recommended if columns are resizable.)");
DEFINE_ENUM(ImGui, TableFlags_PreciseWidths,
R"(Disable distributing remainder width to stretched columns (width allocation
   on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this
   flag: 33,33,33).
   With larger number of columns, resizing will appear to be less smooth.)");

API_SECTION_P(tableFlags, "Clipping");
DEFINE_ENUM(ImGui, TableFlags_NoClip,
R"(Disable clipping rectangle for every individual columns
   (reduce draw command count, items will be able to overflow into other columns).
   Generally incompatible with TableSetupScrollFreeze.)");

API_SECTION_P(tableFlags, "Padding");
DEFINE_ENUM(ImGui, TableFlags_PadOuterX,
R"(Default if TableFlags_BordersOuterV is on. Enable outermost padding.
   Generally desirable if you have headers.)");
DEFINE_ENUM(ImGui, TableFlags_NoPadOuterX,
  "Default if TableFlags_BordersOuterV is off. Disable outermost padding.");
DEFINE_ENUM(ImGui, TableFlags_NoPadInnerX,
R"(Disable inner padding between columns (double inner padding if
   TableFlags_BordersOuterV is on, single inner padding if BordersOuterV is off).)");

API_SECTION_P(tableFlags, "Scrolling");
DEFINE_ENUM(ImGui, TableFlags_ScrollX,
R"(Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable to
   specify the container size. Changes default sizing policy.
   Because this creates a child window, ScrollY is currently generally
   recommended when using ScrollX.)");
DEFINE_ENUM(ImGui, TableFlags_ScrollY,
R"(Enable vertical scrolling.
   Require 'outer_size' parameter of BeginTable to specify the container size.)");

API_SECTION_P(tableFlags, "Sorting");
DEFINE_ENUM(ImGui, TableFlags_SortMulti,
R"(Hold shift when clicking headers to sort on multiple column.
   TableGetGetSortSpecs may return specs where (SpecsCount > 1).)");
DEFINE_ENUM(ImGui, TableFlags_SortTristate,
R"(Allow no sorting, disable default sorting.
   TableGetColumnSortSpecs may return specs where (SpecsCount == 0).)");
