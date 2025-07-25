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

#include "font.hpp"

#include "context.hpp"
#include "error.hpp"

#include <imgui/imgui.h>
#include <imgui/misc/freetype/imgui_freetype.h>

Font::Font(const char *family, const int flags)
  : m_size {}
{
  if(!resolve(family, flags) && !resolve(SANS_SERIF, flags))
    throw reascript_error {"cannot find a suitable font"};
}

Font::Font(const char *file, const int index, const int flags)
  : m_data {file}, m_index {index}, m_flags {flags}, m_size {}
{
}

Font::Font(std::vector<unsigned char> &&data, const int index, const int flags)
  : m_data {std::move(data)}, m_index {index}, m_flags {flags}, m_size {}
{
}

ImFont *Font::instance(Context *ctx)
{
  return ctx->touch<ImFont>(this);
}

static void uninstall(Context *ctx, ImFont *font)
{
  ctx->IO().Fonts->RemoveFont(font);
}

SubresourceData Font::install(Context *ctx)
{
  ImFontConfig cfg;
  // light hinting solves uneven glyph height on macOS
  cfg.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_LightHinting |
                         ImGuiFreeTypeLoaderFlags_LoadColor;
  if(m_flags & ReaImGuiFontFlags_Bold)
    cfg.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_Bold;
  if(m_flags & ReaImGuiFontFlags_Italic)
    cfg.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_Oblique;
  cfg.FontNo = m_index;

  ImFont *font;
  auto atlas {ctx->IO().Fonts};
  if(const std::string *path {std::get_if<std::string>(&m_data)})
    font = atlas->AddFontFromFileTTF(path->c_str(), m_size, &cfg);
  else {
    cfg.FontDataOwnedByAtlas = false;
    auto &data {std::get<std::vector<unsigned char>>(m_data)};
    font = atlas->AddFontFromMemoryTTF(data.data(), data.size(), m_size, &cfg);
  }

  if(!font) // imgui doesn't report what went wrong
    throw reascript_error {"the font could not be loaded"};

  return {font, &uninstall};
}
