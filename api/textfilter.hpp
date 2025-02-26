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

#ifndef REAIMGUI_TEXTFILTER_HPP
#define REAIMGUI_TEXTFILTER_HPP

#include "../src/resource.hpp"

class TextFilter : public Resource {
public:
  TextFilter(const char *filter);
  void set(const char *filter);
  ImGuiTextFilter *operator->();

  bool attachable(const Context *) const override { return true; }

private:
  ImGuiTextFilter m_filter;
};

API_REGISTER_OBJECT_TYPE(TextFilter);

#endif
