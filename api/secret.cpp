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

// Undocumented & unsupported APIs. Use the functions below at your own risks.

#define DEFINE_SECRET_API(type, name, args, ...)           \
  type API_##name(_FOREACH_ARG(_SIGARG, _, args)) noexcept \
  __VA_ARGS__                                              \
                                                           \
  static const API API_reg_##name                          \
    { #name, reinterpret_cast<void *>(&API_##name),        \
      nullptr, nullptr, nullptr, 0 };

DEFINE_SECRET_API(bool, CheckVersionAndDataLayout, (const char*,version)
(size_t,sz_io)(size_t,sz_style)(size_t,sz_vec2)(size_t,sz_vec4)(size_t,sz_vert)
(size_t,sz_idx)(const char**,p_error),
{
  try {
    return ImGui::DebugCheckVersionAndDataLayout(
      version, sz_io, sz_style, sz_vec2, sz_vec4, sz_vert, sz_idx);
  }
  catch(const imgui_error &e) {
    if(p_error)
      *p_error = e.what();
    return false;
  }
});

DEFINE_SECRET_API(void, GetAllocatorFunctions,
(ImGuiMemAllocFunc*,p_alloc_func)(ImGuiMemFreeFunc*,p_free_func)
(void**,p_user_data),
{
  ImGui::GetAllocatorFunctions(p_alloc_func, p_free_func, p_user_data);
});

DEFINE_SECRET_API(ImGuiContext*, GetInternalContext, (ImGui_Context*,ctx)
(bool,enterFrame),
{
  if(!Resource::isValid(ctx))
    return nullptr;

  if(enterFrame)
    ctx->enterFrame();

  return ctx->imgui();
});
