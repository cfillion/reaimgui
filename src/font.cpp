/* ReaImGui: ReaScript binding for Dear ImGui
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

#include "font.hpp"

#include <cassert>
#include <imgui/imgui.h>
#include <imgui/misc/freetype/imgui_freetype.h>

Font::Font(const char *family, const int size, const int flags)
  : m_size { size }
{
  const int style { flags & ReaImGuiFontFlags_StyleMask };
  if(strpbrk(family, "/\\") || !resolve(family, style)) {
    m_data = family;
    m_index = flags & ReaImGuiFontFlags_IndexMask;
    m_missingStyles = style;
  }
}

ImFont *Font::load() try
{
  ImFontConfig cfg;
  // light hinting solves uneven glyph height on macOS
  cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
  if(m_missingStyles & ReaImGuiFontFlags_Bold)
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
  if(m_missingStyles & ReaImGuiFontFlags_Italic)
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Oblique;
  cfg.FontNo = m_index;

  ImGuiIO &io { ImGui::GetIO() };
  const float scale { io.DisplayFramebufferScale.x };
  const int scaledSize { static_cast<int>(m_size * scale) };

  ImFont *font;
  if(const std::string *path { std::get_if<std::string>(&m_data) })
    font = io.Fonts->AddFontFromFileTTF(path->c_str(), scaledSize, &cfg);
  else {
    cfg.FontDataOwnedByAtlas = false;
    auto &data { std::get<std::vector<unsigned char>>(m_data) };
    font = io.Fonts->AddFontFromMemoryTTF(data.data(), data.size(), scaledSize, &cfg);
  }

  font->Scale = 1.f / scale;

  return font;
}
catch(const imgui_error &)
{
  return nullptr;
}

FontList::FontList()
  : m_loaded { false }
{
}

void FontList::add(Font *font)
{
  for(const FontAttachment &attachment : m_fonts) {
    if(attachment.descriptor == font)
      return; // the font was already added
  }

  m_fonts.push_back({ font });
  m_loaded = false;
}

void FontList::keepAliveAll()
{
  for(const FontAttachment &attachment : m_fonts)
    attachment.descriptor->keepAlive();
}

void FontList::loadAll()
{
  ImGuiIO &io { ImGui::GetIO() };

  ImFontConfig cfg;
  cfg.SizePixels = 13.f * io.DisplayFramebufferScale.x;
  ImFont *defFont { io.Fonts->AddFontDefault(&cfg) };
  defFont->Scale = 1.f / io.DisplayFramebufferScale.x;

  for(FontAttachment &attachment : m_fonts)
    attachment.instance = attachment.descriptor->load();

  m_loaded = true;
}

Font *FontList::get(ImFont *instance) const
{
  for(const FontAttachment &attachment : m_fonts) {
    if(attachment.instance == instance)
      return attachment.descriptor;
  }
  return nullptr; // not found, it's probably the default font
}

ImFont *FontList::instanceOf(Font *descriptor) const
{
  if(!descriptor)
    return nullptr; // default font

  for(const FontAttachment &attachment : m_fonts) {
    if(attachment.descriptor == descriptor) {
      assert(attachment.instance && "attached font was not loaded");
      return attachment.instance;
    }
  }

  throw reascript_error { "font is not attached to the context (did you call AttachFont?)" };
}
