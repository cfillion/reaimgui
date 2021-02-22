#include "api_helper.hpp"

DEFINE_API(bool, BeginTable, (ImGui_Context*,ctx)
(const char*,strId)(int, column)(int*,API_RO(flags))
(double*,API_RO(outerWidth))(double*,API_RO(outerHeight))
(double*,API_RO(innerWidth)),
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
       TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
       TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
                         TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
       TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
       --------------------------------------------------------------------------------------------------------
- 5. Call EndTable())",
{
  Context::check(ctx)->enterFrame();

  return ImGui::BeginTable(strId, column,
    valueOr(API_RO(flags), 0),
    ImVec2(
      valueOr(API_RO(outerWidth), 0.0),
      valueOr(API_RO(outerHeight), 0.0)
    ),
    valueOr(API_RO(innerWidth), 0.0)
  );
});

DEFINE_API(void, EndTable, (ImGui_Context*,ctx),
"Only call EndTable() if BeginTable() returns true!",
{
  Context::check(ctx)->enterFrame();
  ImGui::EndTable();
});

DEFINE_API(void, TableNextRow, (ImGui_Context*,ctx)
(int*,API_RO(rowFlags))(double*,API_RO(minRowHeight)),
R"(Append into the first cell of a new row.

Default values: rowFlags = ImGui_TableRowFlags_None, minRowHeight = 0.0)",
{
  Context::check(ctx)->enterFrame();
  ImGui::TableNextRow(valueOr(API_RO(rowFlags), ImGuiTableRowFlags_None),
    valueOr(API_RO(minRowHeight), 0.0));
});

DEFINE_API(bool, TableNextColumn, (ImGui_Context*,ctx),
"Append into the next column (or first column of next row if currently in last column). Return true when column is visible.",
{
  Context::check(ctx)->enterFrame();
  return ImGui::TableNextColumn();
});

    // IMGUI_API bool          TableSetColumnIndex(int column_n);          // append into the specified column. Return true when column is visible.
    // // Tables: Headers & Columns declaration
    // // - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
    // // - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
    // //   Headers are required to perform: reordering, sorting, and opening the context menu.
    // //   The context menu can also be made available in columns body using ImGuiTableFlags_ContextMenuInBody.
    // // - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
    // //   some advanced use cases (e.g. adding custom widgets in header row).
    // // - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
    // IMGUI_API void          TableSetupColumn(const char* label, ImGuiTableColumnFlags flags = 0, float init_width_or_weight = 0.0f, ImU32 user_id = 0);
    // IMGUI_API void          TableSetupScrollFreeze(int cols, int rows); // lock columns/rows so they stay visible when scrolled.
    // IMGUI_API void          TableHeadersRow();                          // submit all headers cells based on data provided to TableSetupColumn() + submit context menu
    // IMGUI_API void          TableHeader(const char* label);             // submit one header cell manually (rarely used)
    // // Tables: Sorting
    // // - Call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
    // // - When 'SpecsDirty == true' you should sort your data. It will be true when sorting specs have changed
    // //   since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting, else you may
    // //   wastefully sort your data every frame!
    // // - Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
    // IMGUI_API ImGuiTableSortSpecs* TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).
    // // Tables: Miscellaneous functions
    // // - Functions args 'int column_n' treat the default value of -1 as the same as passing the current column index.
    // IMGUI_API int                   TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
    // IMGUI_API int                   TableGetColumnIndex();                      // return current column index.
    // IMGUI_API int                   TableGetRowIndex();                         // return current row index.
    // IMGUI_API const char*           TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
    // IMGUI_API ImGuiTableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
    // IMGUI_API void                  TableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n = -1);  // change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details.
    //
    // // Legacy Columns API (2020: prefer using Tables!)
    // // - You can also use SameLine(pos_x) to mimic simplified columns.
    // IMGUI_API void          Columns(int count = 1, const char* id = NULL, bool border = true);
    // IMGUI_API void          NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
    // IMGUI_API int           GetColumnIndex();                                                   // get current column index
    // IMGUI_API float         GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
    // IMGUI_API void          SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
    // IMGUI_API float         GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
    // IMGUI_API void          SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
    // IMGUI_API int           GetColumnsCount();
    //
    // // Tab Bars, Tabs
    // IMGUI_API bool          BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);        // create and append into a TabBar
    // IMGUI_API void          EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
    // IMGUI_API bool          BeginTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
    // IMGUI_API void          EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
    // IMGUI_API bool          TabItemButton(const char* label, ImGuiTabItemFlags flags = 0);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
    // IMGUI_API void          SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.
