/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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
