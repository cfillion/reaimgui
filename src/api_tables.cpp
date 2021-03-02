/* ReaImGui: ReaScript binding for dear imgui
 * Copyright (C) 2021  Christian Fillion
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

#include "api_helper.hpp"

DEFINE_API(bool, BeginTable, (ImGui_Context*,ctx)
(const char*,str_id)(int, column)(int*,API_RO(flags))
(double*,API_RO(outer_size_w))(double*,API_RO(outer_size_h))
(double*,API_RO(inner_width)),
R"([BETA API] API may evolve slightly! If you use this, please update to the next version when it comes out!
- Full-featured replacement for old Columns API.
- See Demo->Tables for demo code.
- See top of imgui_tables.cpp for general commentary.
- See ImGuiTableFlags_ and ImGuiTableColumnFlags_ enums for a description of available flags.
The typical call flow is:
- 1. Call BeginTable().
- 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
- 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of columns/rows.
- 4. Optionally call TableHeadersRow() to submit a header row. Names are pulled from TableSetupColumn() data.
- 5. Populate contents:
   - In most situations you can use TableNextRow() + TableSetColumnIndex(N) to start appending into a column.
   - If you are using tables as a sort of grid, where every columns is holding the same type of contents,
     you may prefer using TableNextColumn() instead of TableNextRow() + TableSetColumnIndex().
     TableNextColumn() will automatically wrap-around into the next row if needed.
   - IMPORTANT: Comparatively to the old Columns() API, we need to call TableNextColumn() for the first column!
   - Summary of possible call flow:
       --------------------------------------------------------------------------------------------------------
       TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
       TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
                         TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
       TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
       --------------------------------------------------------------------------------------------------------
- 5. Call EndTable()

Default values: flags = ImGui_TableFlags_None, outer_size_w = 0.0, outer_size_h = 0.0, inner_width = 0.0)",
{
  FRAME_GUARD;

  return ImGui::BeginTable(str_id, column,
    valueOr(API_RO(flags), 0),
    ImVec2(
      valueOr(API_RO(outer_size_w), 0.0),
      valueOr(API_RO(outer_size_h), 0.0)
    ),
    valueOr(API_RO(inner_width), 0.0)
  );
});

DEFINE_API(void, EndTable, (ImGui_Context*,ctx),
"Only call EndTable() if BeginTable() returns true!",
{
  FRAME_GUARD;
  ImGui::EndTable();
});

DEFINE_API(void, TableNextRow, (ImGui_Context*,ctx)
(int*,API_RO(row_flags))(double*,API_RO(min_row_height)),
R"(Append into the first cell of a new row.

Default values: row_flags = ImGui_TableRowFlags_None, min_row_height = 0.0)",
{
  FRAME_GUARD;
  ImGui::TableNextRow(valueOr(API_RO(row_flags), ImGuiTableRowFlags_None),
    valueOr(API_RO(min_row_height), 0.0));
});

DEFINE_API(bool, TableNextColumn, (ImGui_Context*,ctx),
"Append into the next column (or first column of next row if currently in last column). Return true when column is visible.",
{
  FRAME_GUARD;
  return ImGui::TableNextColumn();
});

DEFINE_API(bool, TableSetColumnIndex, (ImGui_Context*,ctx)
(int,column_n),
"Append into the specified column. Return true when column is visible.",
{
  FRAME_GUARD;
  return ImGui::TableSetColumnIndex(column_n);
});
    // // Tables: Headers & Columns declaration
    // // - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
    // //   Headers are required to perform: reordering, sorting, and opening the context menu.
    // //   The context menu can also be made available in columns body using ImGuiTableFlags_ContextMenuInBody.
    // // - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
    // //   some advanced use cases (e.g. adding custom widgets in header row).
    // // - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
DEFINE_API(void, TableSetupColumn, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RO(flags))(double*,API_RO(init_width_or_weight))
(int*,API_RO(user_id)),
R"(Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.

Default values: flags = ImGui_TableColumnFlags_None, init_width_or_weight = 0.0, user_id = 0)",
{
  FRAME_GUARD;
  ImGui::TableSetupColumn(label,
    valueOr(API_RO(flags), ImGuiTableColumnFlags_None),
    valueOr(API_RO(init_width_or_weight), 0.0), valueOr(API_RO(user_id), 0));
});

DEFINE_API(void, TableSetupScrollFreeze, (ImGui_Context*,ctx)
(int,cols)(int,rows),
"Lock columns/rows so they stay visible when scrolled.",
{
  FRAME_GUARD;
  ImGui::TableSetupScrollFreeze(cols, rows);
});

DEFINE_API(void, TableHeadersRow, (ImGui_Context*,ctx),
"Submit all headers cells based on data provided to TableSetupColumn() + submit context menu",
{
  FRAME_GUARD;
  ImGui::TableHeadersRow();
});

DEFINE_API(void, TableHeader, (ImGui_Context*,ctx)
(const char*,label),
"Submit one header cell manually (rarely used). See ImGui_TableSetColumn",
{
  FRAME_GUARD;
  ImGui::TableHeader(label);
});

DEFINE_API(bool, TableNeedSort, (ImGui_Context*,ctx)
(bool*,API_W(has_specs)),
"Return true once when sorting specs have changed since last call, or the first time. 'has_specs' is false when not sorting. See ImGui_TableSortSpecs_GetColumnSpecs.",
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
});

DEFINE_API(bool, TableGetColumnSortSpecs, (ImGui_Context*,ctx)
(int,id)
(int*,API_W(column_user_id))(int*,API_W(column_index))
(int*,API_W(sort_order))(int*,API_W(sort_direction)),
R"(Sorting specification for one column of a table. Call while incrementing id from 0 until false is returned.

ColumnUserID:  User id of the column (if specified by a TableSetupColumn() call)
ColumnIndex:   Index of the column
SortOrder:     Index within parent ImGuiTableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
SortDirection: ImGuiSortDirection_Ascending or ImGuiSortDirection_Descending (you can use this or SortSign, whichever is more convenient for your sort function)

See ImGui_TableNeedSort.)",
{
  FRAME_GUARD;

  const ImGuiTableSortSpecs *specs { ImGui::TableGetSortSpecs() };
  if(!specs || id >= specs->SpecsCount)
    return false;

  const ImGuiTableColumnSortSpecs &spec { specs->Specs[id] };
  if(API_W(column_user_id)) *API_W(column_user_id) = spec.ColumnUserID;
  if(API_W(column_index))   *API_W(column_index)   = spec.ColumnIndex;
  if(API_W(sort_order))     *API_W(sort_order)     = spec.SortOrder;
  if(API_W(sort_direction)) *API_W(sort_direction) = spec.SortDirection;
  return true;
});

DEFINE_API(int, TableGetColumnCount, (ImGui_Context*,ctx),
"Return number of columns (value passed to BeginTable)",
{
  FRAME_GUARD;
  return ImGui::TableGetColumnCount();
});

DEFINE_API(int, TableGetColumnIndex, (ImGui_Context*,ctx),
"Return current column index.",
{
  FRAME_GUARD;
  return ImGui::TableGetColumnIndex();
});

DEFINE_API(int, TableGetRowIndex, (ImGui_Context*,ctx),
"Return current row index.",
{
  FRAME_GUARD;
  return ImGui::TableGetRowIndex();
});

DEFINE_API(const char*, TableGetColumnName, (ImGui_Context*,ctx)
(int*,API_RO(column_n)),
R"(Return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.

Default values: column_n = -1)",
{
  FRAME_GUARD;
  return ImGui::TableGetColumnName(valueOr(API_RO(column_n), -1));
});

DEFINE_API(int, TableGetColumnFlags, (ImGui_Context*,ctx)
(int*,API_RO(column_n)),
R"(Return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.

Default values: column_n = -1)",
{
  FRAME_GUARD;
  return ImGui::TableGetColumnFlags(valueOr(API_RO(column_n), -1));
});

DEFINE_API(void, TableSetBgColor, (ImGui_Context*,ctx)
(int,target)(int,color_rgba)(int*,API_RO(column_n)),
R"(Change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details.

Default values: column_n = -1)",
{
  FRAME_GUARD;
  ImGui::TableSetBgColor(target,
    Color::rgba2abgr(color_rgba), valueOr(API_RO(column_n), -1));
});
