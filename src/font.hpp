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

#ifndef REAIMGUI_FONT_HPP
#define REAIMGUI_FONT_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "resource.hpp"
#include "variant.hpp"

enum FontFlags {
  ReaImGuiFontFlags_None      = 0,
  ReaImGuiFontFlags_IndexMask = 0xFF, // font index when loading from a collection file
  ReaImGuiFontFlags_Bold      = 1<<8,
  ReaImGuiFontFlags_Italic    = 1<<9,
  ReaImGuiFontFlags_StyleMask = ~0xFF,
};

struct ImFont;
struct ImFontAtlas;

class Font : public Resource {
public:
  static constexpr const char *api_type_name { "ImGui_Font" };

  // generic fonts
  static constexpr const char
    *CURSIVE    { "cursive" },
    *FANTASY    { "fantasy" },
    *MONOSPACE  { "monospace" },
    *SANS_SERIF { "sans-serif" },
    *SERIF      { "serif" };

  Font(const char *family, int size, int style);
  ImFont *load(ImFontAtlas *, float scale);

private:
  bool resolve(const char *family, int style);

  std::variant<std::string, std::vector<unsigned char>> m_data;
  int m_index, m_size, m_missingStyles;
};

using ImGui_Font = Font; // user-facing alias

class FontList {
public:
  FontList();
  ~FontList();

  void add(Font *);
  void remove(Font *);
  void keepAliveAll();
  void update();
  int texVersion() const { return m_version; }
  int setScale(float scale); // returns the current texture version
  Font *get(ImFont *) const;
  ImFont *instanceOf(Font *) const;

private:
  void invalidate();
  void build(float scale);
  void migrateActiveFonts();
  ImFont *toCurrentAtlas(ImFont *) const;

  std::vector<Font *> m_fonts;
  std::unordered_map<float, std::unique_ptr<ImFontAtlas>> m_atlases;
  bool m_rebuild;
  int m_version;
};

#endif
