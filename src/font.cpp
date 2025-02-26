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

#include "error.hpp"
#include "texture.hpp"

#include <algorithm>
#include <cassert>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/freetype/imgui_freetype.h>

static const unsigned char *getPixels(
  const Texture &texture, int *width, int *height)
{
  FontList *list {static_cast<FontList *>(texture.object())};
  ImFontAtlas *atlas {list->getAtlas(texture.scale())};
  unsigned char *pixels {};
  atlas->GetTexDataAsRGBA32(&pixels, width, height);
  return pixels;
}

static bool removeScale(const Texture &texture)
{
  FontList *list {static_cast<FontList *>(texture.object())};
  return list->removeAtlas(texture.scale());
}

Font::Font(const char *family, const int size, const int flags)
  : m_size {size}
{
  const int style {flags & ReaImGuiFontFlags_StyleMask};
  if(strpbrk(family, "/\\") || !resolve(family, style)) {
    m_data = family;
    m_index = flags & ReaImGuiFontFlags_IndexMask;
    m_missingStyles = style;
  }
}

Font::Font(std::vector<unsigned char> &&data, const int size, const int flags)
  : m_data {std::move(data)}, m_index {flags & ReaImGuiFontFlags_IndexMask},
    m_size {size}, m_missingStyles {flags & ReaImGuiFontFlags_StyleMask}
{
}

ImFont *Font::load(ImFontAtlas *atlas, const float scale)
{
  ImFontConfig cfg;
  // light hinting solves uneven glyph height on macOS
  cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LightHinting |
                          ImGuiFreeTypeBuilderFlags_LoadColor;
  if(m_missingStyles & ReaImGuiFontFlags_Bold)
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
  if(m_missingStyles & ReaImGuiFontFlags_Italic)
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Oblique;
  cfg.FontNo = m_index;

  const int scaledSize {static_cast<int>(m_size * scale)};
  cfg.RasterizerDensity = static_cast<float>(scaledSize) / m_size;

  ImFont *font;
  if(const std::string *path {std::get_if<std::string>(&m_data)})
    font = atlas->AddFontFromFileTTF(path->c_str(), scaledSize, &cfg);
  else {
    cfg.FontDataOwnedByAtlas = false;
    auto &data {std::get<std::vector<unsigned char>>(m_data)};
    font = atlas->AddFontFromMemoryTTF(data.data(), data.size(), scaledSize, &cfg);
  }
  font->Scale = static_cast<float>(m_size) / scaledSize;

  return font;
}

FontList::FontList(TextureManager *manager)
  : m_textureManager {manager}, m_rebuild {false}
{
}

FontList::~FontList()
{
  for(auto &pair : m_atlases)
    pair.second->Locked = false;
}

void FontList::invalidate()
{
  m_rebuild = !m_atlases.empty(); // don't rebuild before the first frame
}

void FontList::add(Font *font)
{
  if(std::find(m_fonts.begin(), m_fonts.end(), font) != m_fonts.end())
    return;

  m_fonts.push_back(font);
  invalidate();
}

void FontList::remove(Font *font)
{
  const auto it {std::find(m_fonts.begin(), m_fonts.end(), font)};
  if(it == m_fonts.end())
    return;

  m_fonts.erase(it);
  invalidate();
}

void FontList::update()
{
  if(m_atlases.empty())
    setScale(ImGui::GetPlatformIO().Monitors[0].DpiScale);

  if(m_rebuild) {
    for(const auto &pair : m_atlases) {
      // EndFrame unlocks only the current atlas in io.Fonts
      pair.second->Locked = false;
      build(pair.first);
    }
    m_textureManager->invalidate(this);
    m_rebuild = false;
  }
}

void FontList::setScale(const float scale)
{
  ImGuiIO &io {ImGui::GetIO()};

  std::unique_ptr<ImFontAtlas> &atlas {m_atlases[scale]};
  if(!atlas)
    atlas.reset(new ImFontAtlas);

  const bool atlasChanged {atlas.get() != io.Fonts};
  io.Fonts = atlas.get();

  if(!atlas->IsBuilt())
    build(scale);

  if(atlasChanged)
    migrateActiveFonts();

  // after build() because ImFontAtlasBuildWithFreeTypeEx clears atlas->TexId
  atlas->SetTexID(m_textureManager->touch(
    this, scale, &getPixels, nullptr, &removeScale));
}

ImFontAtlas *FontList::getAtlas(const float scale)
{
  const auto it {m_atlases.find(scale)};
  return it != m_atlases.end() ? it->second.get() : nullptr;
}

bool FontList::removeAtlas(const float scale)
{
  const float primaryScale {ImGui::GetPlatformIO().Monitors[0].DpiScale};
  if(scale == primaryScale)
    return false;

  const auto it {m_atlases.find(scale)};
  if(it == m_atlases.end())
    return true; // let the texture manager free it

  ImGuiIO &io {ImGui::GetIO()};
  if(io.Fonts == it->second.get())
    io.Fonts = m_atlases[primaryScale].get();

  it->second->Locked = false;
  m_atlases.erase(it);
  return true;
}

void FontList::build(const float scale)
try {
  auto &atlas {m_atlases.at(scale)}; // don't insert
  atlas->ClearFonts();

  ImFontConfig cfg;
  cfg.SizePixels = 13.f * scale;
  ImFont *defFont {atlas->AddFontDefault(&cfg)};
  defFont->Scale = 1.f / scale;

  for(Font *font : m_fonts)
    font->load(atlas.get(), scale);

  atlas->Flags |= ImFontAtlasFlags_NoMouseCursors;
  atlas->Build();
  atlas->ClearInputData();
}
catch(const imgui_error &e) {
  throw imgui_error {"cannot build the font atlas: {}", e.what()};
}

void FontList::migrateActiveFonts()
{
  if(ImFont *currentFont {ImGui::GetFont()})
    ImGui::SetCurrentFont(toCurrentAtlas(currentFont));

  auto &fontStack {ImGui::GetCurrentContext()->FontStack};
  for(int i {}; i < fontStack.Size; ++i)
    fontStack[i] = toCurrentAtlas(fontStack[i]);
}

Font *FontList::get(ImFont *instance) const
{
  const ImFontAtlas *atlas {ImGui::GetIO().Fonts};
  for(int i {1}; i < atlas->Fonts.Size; ++i) {
    assert(static_cast<size_t>(i) <= m_fonts.size());

    if(atlas->Fonts[i] == instance)
      return m_fonts[i - 1];
  }
  return nullptr; // default font
}

ImFont *FontList::instanceOf(Font *font) const
{
  if(!font)
    return nullptr; // default font

  const auto it {std::find(m_fonts.begin(), m_fonts.end(), font)};
  if(it == m_fonts.end())
    throw reascript_error {"font is not attached to the context"};

  const auto index {std::distance(m_fonts.begin(), it) + 1};
  const ImFontAtlas *atlas {ImGui::GetIO().Fonts};
  assert(index < atlas->Fonts.Size);
  return atlas->Fonts[index];
}

ImFont *FontList::toCurrentAtlas(ImFont *oldInstance) const
{
  const ImFontAtlas *newAtlas {ImGui::GetIO().Fonts},
                    *oldAtlas {oldInstance->ContainerAtlas};

  if(newAtlas == oldAtlas)
    return oldInstance;

  const int size {std::min(oldAtlas->Fonts.Size, newAtlas->Fonts.Size)};
  for(int i {0}; i < size; ++i) {
    if(oldAtlas->Fonts[i] == oldInstance)
      return newAtlas->Fonts[i];
  }

  return ImGui::GetDefaultFont();
}
