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

#include <imgui/imgui_internal.h>
#include <imgui/misc/freetype/imgui_freetype.h>

static void lookupFallbackFont(ImFontAtlas *atlas, ImFont *inst, const ImWchar c)
{
  if(c == '\t')
    return;

  // imgui caches the result per baked font (per size per ctx)
  auto font {static_cast<Font *>(inst->Sources.front()->UserData)};
  if(SysFont *sysfont {dynamic_cast<SysFont *>(font)})
    sysfont->addFallback(atlas, inst, c);
}

const ImFontLoader *Font::loader()
{
  static auto loader {*ImGuiFreeType::GetFontLoader()};
  loader.FontAddFallbackSrc = &lookupFallbackFont;
  return &loader;
}

Font::Font()
  : m_size {}
{
}

Font::Font(const char *file, const unsigned int index, const int flags)
  : m_src {file, index, flags}, m_size {}
{
}

Font::Font(std::vector<unsigned char> &&data, const unsigned int index, const int flags)
  : m_src {std::move(data), index, flags}, m_size {}
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
  if(ImFont *inst {m_src.install(ctx->IO().Fonts, this)})
    return {inst, &uninstall};

  // imgui doesn't report what went wrong
  throw reascript_error {"the font could not be loaded"};
}

SysFont::SysFont(const char *family, const int flags)
  : Font {}, m_family {family}, m_styles {flags}
{
  if(auto src {resolve()})
    m_src = *src;
  else
    throw reascript_error {"cannot find a matching system font"};
}

bool SysFont::addFallback(ImFontAtlas *atlas, ImFont *inst, unsigned int codepoint)
{
  if(const auto src {resolve(codepoint)}) {
    if(*src != m_src)
      return src->install(atlas, this, inst) != nullptr;
  }
  return false;
}

ImFont *FontSource::install(ImFontAtlas *atlas, Font *parent, ImFont *inst) const
{
  ImFontConfig cfg;
  // light hinting solves uneven glyph height on macOS
  cfg.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_LightHinting |
                         ImGuiFreeTypeLoaderFlags_LoadColor;
  if(m_styles & ReaImGuiFontFlags_Bold)
    cfg.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_Bold;
  if(m_styles & ReaImGuiFontFlags_Italic)
    cfg.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_Oblique;
  cfg.FontNo = m_index;
  cfg.UserData = parent;
  cfg.MergeTarget = inst;

  const auto size {parent->legacySize()};
  if(const std::string *path {std::get_if<std::string>(&m_data)})
    return atlas->AddFontFromFileTTF(path->c_str(), size, &cfg);
  else {
    cfg.FontDataOwnedByAtlas = false;
    auto &data {std::get<std::vector<unsigned char>>(m_data)};
    auto bytes {const_cast<unsigned char *>(data.data())};
    return atlas->AddFontFromMemoryTTF(bytes, data.size(), size, &cfg);
  }
}

bool FontSource::operator==(const FontSource &o) const
{
  return m_data == o.m_data && m_index == o.m_index && m_styles == o.m_styles;
}
