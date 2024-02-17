#include "shims.hpp"

SHIM("0.9",
  (bool, TableGetColumnSortSpecs, ImGui_Context*, int, int*, int*, int*)
);

// TODO: CreateDrawListSplitter, CreateImage, CreateImageFromMem,
// CreateImageSet, CreateTextFilter, CreateListClipper, CreateFunctionFromEEL,
// CreateFont

SHIM_FUNC(0_1, void, DestroyContext, (ImGui_Context*,)) {} // no-op

// Removed the redundant sort_order return value (always same as id)
// and swapped the column_{index,user_id} arguments
SHIM_FUNC(0_1, bool, TableGetColumnSortSpecs, (ImGui_Context*,ctx)
(int,id)(int*,API_W(column_user_id))(int*,API_W(column_index))
(int*,API_W(sort_order))(int*,API_W(sort_direction)))
{
  if(API_W(sort_order)) *API_W(sort_order) = id;
  return api.TableGetColumnSortSpecs(ctx, id, API_W(column_index), API_W(column_user_id), API_W(sort_direction));
}
