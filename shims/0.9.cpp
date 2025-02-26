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

#include "shims.hpp"

#include "../api/drawlist.hpp"
#include "../api/listclipper.hpp"
#include "../api/textfilter.hpp"
#include "../src/font.hpp"
#include "../src/function.hpp"
#include "../src/image.hpp"
#include "../src/keymap.hpp"

#include <imgui/imgui_internal.h>

SHIM("0.9",
  (DrawListSplitter*, CreateDrawListSplitter, DrawListProxy*)
  (Font*,        CreateFont, const char*, int, RO<int*>)
  (Function*,    CreateFunctionFromEEL, const char*)
  (Image*,       CreateImage, const char*, RO<int*>)
  (Image*,       CreateImageFromMem, const char*, S<int>)
  (ImageSet*,    CreateImageSet)
  (ListClipper*, CreateListClipper, Context*)
  (TextFilter*,  CreateTextFilter, RO<const char*>)

  (bool, TableGetColumnSortSpecs, Context*, int, W<int*>, W<int*>, W<int*>)

  (int,  SelectableFlags_AllowOverlap)
  (int,  TreeNodeFlags_AllowOverlap)
  (int,  HoveredFlags_AllowWhenOverlappedByItem)
  (bool, IsItemHovered,   Context*, RO<int*>)
  (bool, IsWindowHovered, Context*, RO<int*>)

  (void, ListClipper_IncludeItemsByIndex, ListClipper*, int, int)
  (bool, BeginTable, Context*, const char*, int,
    RO<int*>, RO<double*>, RO<double*>, RO<double*>)
  (void, TableNextRow, Context*, RO<int*>, RO<double*>)

  (void, ShowIDStackToolWindow, Context*, RWO<bool*>)

  (int,  ChildFlags_Border)
  (int,  ChildFlags_FrameStyle)
  (bool, BeginChild, Context*,
    const char*, RO<double*>, RO<double*>, RO<int*>, RO<int*>)
  (void, EndChild, Context*)

  (bool,   IsKeyDown, Context*, int)
  (double, GetKeyDownDuration, Context*, int)
  (bool,   IsKeyPressed, Context*, int, RO<bool*>)
  (bool,   IsKeyReleased, Context*, int)
  (int,    GetKeyPressedAmount, Context*, int, double, double)
);

SHIM_PROXY_BEGIN(CreateExemptGCCheck, func, args)
{
  Resource::bypassGCCheckOnce();
  return std::apply(api.*func, args);
}
SHIM_PROXY_END()
SHIM_PROXY(0_7_1, CreateDrawListSplitter, CreateExemptGCCheck)
SHIM_PROXY(0_4,   CreateFont,             CreateExemptGCCheck)
SHIM_PROXY(0_8_5, CreateFunctionFromEEL,  CreateExemptGCCheck)
SHIM_PROXY(0_8,   CreateImage,            CreateExemptGCCheck)
SHIM_PROXY(0_8,   CreateImageFromMem,     CreateExemptGCCheck)
SHIM_PROXY(0_8,   CreateImageSet,         CreateExemptGCCheck)
SHIM_PROXY(0_1,   CreateListClipper,      CreateExemptGCCheck)
SHIM_PROXY(0_5_6, CreateTextFilter,       CreateExemptGCCheck)

SHIM_FUNC(0_1, void, DestroyContext, (Context*,)) {} // no-op

// Removed the redundant sort_order return value (always same as id)
// and swapped the column_{index,user_id} arguments
SHIM_FUNC(0_1, bool, TableGetColumnSortSpecs, (Context*,ctx)
  (int,id) (W<int*>,column_user_id) (W<int*>,column_index)
  (W<int*>,sort_order) (W<int*>,sort_direction))
{
  if(sort_order) *sort_order = id;
  return api.TableGetColumnSortSpecs(ctx, id,
    column_index, column_user_id, sort_direction);
}

// dear imgui 1.89.7
SHIM_FUNC(0_1, void, SetItemAllowOverlap, (Context*,ctx)) {} // always been broken
SHIM_ALIAS(0_1, SelectableFlags_AllowItemOverlap, SelectableFlags_AllowOverlap)
SHIM_ALIAS(0_1, TreeNodeFlags_AllowItemOverlap,   TreeNodeFlags_AllowOverlap)
SHIM_FUNC(0_1, bool, IsItemHovered, (Context*,ctx) (RO<int*>,flags))
{
  if(flags) {
    *flags &= ImGuiHoveredFlags_AllowedMaskForIsItemHovered;
    *flags |= api.HoveredFlags_AllowWhenOverlappedByItem();
  }
  return api.IsItemHovered(ctx, flags);
}
SHIM_FUNC(0_1, bool, IsWindowHovered, (Context*,ctx) (RO<int*>,flags))
{
  if(flags)
    *flags &= ImGuiHoveredFlags_AllowedMaskForIsWindowHovered;
  return api.IsWindowHovered(ctx, flags);
}

// dear imgui 1.89.9
SHIM_ALIAS(0_8_7, ListClipper_IncludeRangeByIndices, ListClipper_IncludeItemsByIndex);

// disable per-row CellPadding.y because it breaks existing layouts
SHIM_FUNC(0_1, bool, BeginTable, (Context*,ctx) (const char*,str_id)
  (int,column) (RO<int*>,flags) (RO<double*>,outer_size_w)
  (RO<double*>,outer_size_h) (RO<double*>,inner_width))
{
  if(!api.BeginTable(ctx, str_id, column, flags,
      outer_size_w, outer_size_h, inner_width))
    return false;
  if(ImGuiTable *table {ctx->imgui()->CurrentTable})
    table->RowCellPaddingY = ctx->style().CellPadding.y;
  return true;
}
SHIM_FUNC(0_8, void, TableNextRow, (Context*,ctx) (RO<int*>,row_flags)
  (RO<double*>,min_row_height))
{
  FRAME_GUARD;
  ImGuiStyle &style {ctx->style()};
  const float backup_y {style.CellPadding.y};
  if(const ImGuiTable *table {ctx->imgui()->CurrentTable})
    style.CellPadding.y = table->RowCellPaddingY;
  api.TableNextRow(ctx, row_flags, min_row_height);
  style.CellPadding.y = backup_y;
}

// dear imgui 1.90
SHIM_ALIAS(0_5_10, ShowStackToolWindow, ShowIDStackToolWindow);
SHIM_FUNC(0_1, bool, BeginChild, (Context*,ctx) (const char*,str_id)
  (RO<double*>,size_w) (RO<double*>,size_h) (RO<bool*>,border) (RO<int*>,flags))
{
  int child_flags {};
  if(border && *border)
    child_flags |= api.ChildFlags_Border();
  return api.BeginChild(ctx, str_id, size_w, size_h, &child_flags, flags);
}
SHIM_FUNC(0_3, bool, BeginChildFrame, (Context*,ctx) (const char*,str_id)
  (double,size_w) (double,size_h) (RO<int*>,window_flags))
{
  int child_flags {api.ChildFlags_FrameStyle()};
  return api.BeginChild(ctx, str_id, &size_w, &size_h, &child_flags, window_flags);
}
SHIM_FUNC(0_8, void, EndChildFrame, (Context*,ctx))
{
  api.EndChild(ctx);
}
SHIM_CONST(0_1, WindowFlags_AlwaysUseWindowPadding, 0)
SHIM_PROXY_BEGIN(ShimVirtualKeys, func, args)
{
  int &key {std::get<1>(args)};
  if(ImGui::IsLegacyKey(static_cast<ImGuiKey>(key))) {
    if(!(key = KeyMap::translateVirtualKey(key)))
      return decltype(std::apply(api.*func, args)) {};
  }
  return std::apply(api.*func, args);
}
SHIM_PROXY_END()
SHIM_PROXY(0_1, IsKeyDown,           ShimVirtualKeys)
SHIM_PROXY(0_1, GetKeyDownDuration,  ShimVirtualKeys)
SHIM_PROXY(0_1, IsKeyPressed,        ShimVirtualKeys)
SHIM_PROXY(0_1, IsKeyReleased,       ShimVirtualKeys)
SHIM_PROXY(0_1, GetKeyPressedAmount, ShimVirtualKeys)
