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

#include "helper.hpp"

#include "textfilter.hpp"

TextFilter::TextFilter(const char *filter)
  : m_filter {filter}
{
}

void TextFilter::set(const char *filter)
{
  m_filter = filter;
  m_filter.Build();
}

ImGuiTextFilter *TextFilter::operator->()
{
  assertValid(this);
  return &m_filter;
}

API_SECTION("Text Filter",
R"(Helper to perform simple filtering on text strings.
In format "aaaaa[,bbbb][,ccccc]".

Filter usage:
- ""         display all lines
- "xxx"      display lines containing "xxx"
- "xxx,yyy"  display lines containing "xxx" or "yyy"
- "-xxx"     hide lines containing "xxx")");

API_FUNC(0_9, TextFilter*, CreateTextFilter,
(RO<const char*>,default_filter,""),
"Valid while used every frame unless attached to a context (see Attach).")
{
  return new TextFilter {API_GET(default_filter)};
}

API_FUNC(0_5_6, void, TextFilter_Set, (TextFilter*,filter)
(const char*,filter_text),
"")
{
  assertValid(filter);
  filter->set(filter_text);
}

API_FUNC(0_5_6, const char*, TextFilter_Get, (TextFilter*,filter),
"")
{
  return (*filter)->InputBuf;
}

API_FUNC(0_5_6, bool, TextFilter_Draw, (TextFilter*,filter)
(Context*,ctx) (RO<const char*>,label,"Filter (inc,-exc)")
(RO<double*>,width,0.0),
"Helper calling InputText+TextFilter_Set")
{
  FRAME_GUARD;

  return (*filter)->Draw(API_GET(label), API_GET(width));
}

API_FUNC(0_5_6, bool, TextFilter_PassFilter, (TextFilter*,filter)
(const char*,text),
"")
{
  return (*filter)->PassFilter(text);
}

API_FUNC(0_5_6, void, TextFilter_Clear, (TextFilter*,filter),
"")
{
  return (*filter)->Clear();
}

API_FUNC(0_5_6, bool, TextFilter_IsActive, (TextFilter*,filter),
"")
{
  return (*filter)->IsActive();
}
