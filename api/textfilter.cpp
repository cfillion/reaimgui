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

#include "textfilter.hpp"

API_SECTION("Text Filter");

TextFilter::TextFilter(const char *filter)
  : m_filter { filter }
{
}

void TextFilter::set(const char *filter)
{
  keepAlive();
  m_filter = filter;
  m_filter.Build();
}

ImGuiTextFilter *TextFilter::operator->()
{
  keepAlive();
  return &m_filter;
}

DEFINE_API(ImGui_TextFilter*, CreateTextFilter,
(const char*,API_RO(default_filter)),
R"(Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]". Valid until unused.

Default values: default_filter = "")",
{
  if(!API_RO(default_filter))
    API_RO(default_filter) = "";

  return new TextFilter { API_RO(default_filter) };
});

DEFINE_API(void, TextFilter_Set, (ImGui_TextFilter*,filter)
(const char*,filter_text),
"",
{
  assertValid(filter);
  filter->set(filter_text);
});

DEFINE_API(const char*, TextFilter_Get, (ImGui_TextFilter*,filter),
"",
{
  assertValid(filter);
  return (*filter)->InputBuf;
});

DEFINE_API(bool, TextFilter_Draw, (ImGui_TextFilter*,filter)
(ImGui_Context*,ctx)(const char*,API_RO(label))(double*,API_RO(width)),
R"~(Helper calling InputText+TextFilter_Set

Default values: label = "Filter (inc,-exc)", width = 0.0)~",
{
  assertValid(filter);
  FRAME_GUARD;

  if(!API_RO(label) || !API_RO(label)[0] /* empty */)
    API_RO(label) = "Filter (inc,-exc)";

  return (*filter)->Draw(API_RO(label), valueOr(API_RO(width), 0.f));
});

DEFINE_API(bool, TextFilter_PassFilter, (ImGui_TextFilter*,filter)
(const char*,text),
"",
{
  assertValid(filter);
  return (*filter)->PassFilter(text);
});

DEFINE_API(void, TextFilter_Clear, (ImGui_TextFilter*,filter),
"",
{
  assertValid(filter);
  return (*filter)->Clear();
});

DEFINE_API(bool, TextFilter_IsActive, (ImGui_TextFilter*,filter),
"",
{
  assertValid(filter);
  return (*filter)->IsActive();
});
