/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

#include "accessibility.hpp"

#include "viewport.hpp"

#include <imgui/imgui_internal.h>

static Viewport *currentViewport(ImGuiContext *ctx)
{
  return static_cast<Viewport *>(ctx->CurrentViewport->PlatformUserData);
}

static Accessibility *currentA11y(ImGuiContext *ctx)
{
  Viewport *viewport { currentViewport(ctx) };
  return viewport ? viewport->accessibility() : nullptr;
}

static std::string_view visibleText(const char *label)
{
  const char *end { ImGui::FindRenderedTextEnd(label) };
  if(const size_t size { static_cast<size_t>(end - label) })
    return { label, size };
  while(*label == '#') ++label;
  return label;
}

void ImGuiTestEngineHook_ItemAdd(ImGuiContext *ctx, const ImGuiID id,
  const ImRect &rect, const ImGuiLastItemData *)
{
  // rect is not always the same as data->NavRect (eg. whole window)
  if(Accessibility *a11y { currentA11y(ctx) }) {
    AccessibilityItem &item { a11y->itemFromID(id, false) };
    item.setRect(rect.ToVec4());
    // item.setData(data); // can be null
  }
}

void ImGuiTestEngineHook_ItemInfo(ImGuiContext *ctx, const ImGuiID id,
  const char *label, const ImGuiItemStatusFlags flags)
{
  if(Accessibility *a11y { currentA11y(ctx) }) {
    const AccessibilityItem::State state
      { a11y, visibleText(label), flags };
    AccessibilityItem &item { a11y->itemFromID(id, true) };
    item.setState(state);
  }
}

void ImGuiTestEngineHook_Log(ImGuiContext *, const char *, ...)
{
}

const char *ImGuiTestEngine_FindItemDebugLabel(ImGuiContext *, const ImGuiID)
{
  return nullptr;
}

Accessibility::Accessibility()
  : m_textIndex {}
{
}

AccessibilityParent Accessibility::findItemParent(const ImGuiID id)
{
  ImGuiContext *ctx { ImGui::GetCurrentContext() };

  const auto begin { m_items.begin() + m_textIndex };
  for(ImGuiWindow *window { ctx->CurrentWindow };
      window; window = window->ParentWindow) {
    const auto &idStack { window->IDStack };

    for(int i { idStack.Size - 1 }; i >= 0; --i) {
      const ImGuiID parentId { idStack[i] };
      if(parentId == id)
        continue;
      const auto it { std::lower_bound(begin, m_items.end(), parentId) };
      if(it != m_items.end() && it->id() == parentId)
        return &*it;
    }
  }

  return currentViewport(ctx);
}

AccessibilityItem &Accessibility::itemFromID(const ImGuiID id, const bool isLastGet)
{
  auto it { m_items.begin() + m_textIndex };

  if(id)
    it = std::lower_bound(it, m_items.end(), id);
  else if(isLastGet)
    ++m_textIndex;

  if(it == m_items.end() || it->id() != id)
    it = m_items.emplace(it, id);

  it->touch();

  return *it;
}

void Accessibility::update()
{
  using namespace std::placeholders;
  const int forgetOlderThan { ImGui::GetFrameCount() - 10 };
  const auto isStale
    { std::bind(&AccessibilityItem::isOlderThan, _1, forgetOlderThan) };

  m_items.erase(
    std::remove_if(m_items.begin(), m_items.end(), isStale),
    m_items.end());

  m_textIndex = 0;
}

AccessibilityItem::AccessibilityItem(const ImGuiID id)
  : m_id { id }, m_lastActiveFrame {}
{
}

void AccessibilityItem::touch()
{
  m_lastActiveFrame = ImGui::GetFrameCount();
}
