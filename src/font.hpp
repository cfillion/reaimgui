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

#ifndef REAIMGUI_FONT_HPP
#define REAIMGUI_FONT_HPP

#include "resource.hpp"

#include <string>
#include <variant>
#include <vector>

enum FontFlags {
  // skip the first 8 bits as they were reserved to font face index until v0.10
  ReaImGuiFontFlags_None      = 0,
  ReaImGuiFontFlags_Bold      = 1<<8,
  ReaImGuiFontFlags_Italic    = 1<<9,

  ReaImGuiFontFlags_IndexMask = 0xFF, // font index when loading from a collection file
  ReaImGuiFontFlags_StyleMask = ~0xFF,
};

struct ImFont;

class Font final : public Resource {
public:
  // generic fonts
  static constexpr const char
    *CURSIVE    {"cursive"},
    *FANTASY    {"fantasy"},
    *MONOSPACE  {"monospace"},
    *SANS_SERIF {"sans-serif"},
    *SERIF      {"serif"};

  Font(const char *family, int style);
  Font(const char *file, int index, int style);
  Font(std::vector<unsigned char> &&, int index, int style);

  bool attachable(const Context *) const override { return true; }
  SubresourceData install(Context *) override;

  ImFont *instance(Context *ctx);
  int legacySize() const { return m_size; }
  void setLegacySize(int sz) { m_size = sz; }

private:
  bool resolve(const char *family, int style);

  std::variant<std::string, std::vector<unsigned char>> m_data;
  int m_index, m_flags, m_size;
};

API_REGISTER_OBJECT_TYPE(Font);

#endif
