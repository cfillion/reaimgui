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

#ifndef REAIMGUI_ACCESSIBILITY_HPP
#define REAIMGUI_ACCESSIBILITY_HPP

#include <string_view>
#include <variant>
#include <vector>

#include <imgui/imgui.h>

class AccessibilityItem;
class Viewport;
using ImGuiItemStatusFlags = int;

using AccessibilityParent = std::variant<Viewport *, AccessibilityItem *>;

class Accessibility {
public:
  Accessibility();

  void update(); // call once per frame

  AccessibilityItem &itemFromID(const ImGuiID, bool isLastGet);
  AccessibilityParent findItemParent(ImGuiID);

private:
  std::vector<AccessibilityItem> m_items;
  size_t m_textIndex;
};

class AccessibilityItem {
public:
  struct State {
    Accessibility *a11y;
    std::string_view label;
    ImGuiItemStatusFlags flags; // ImGuiLastItemData::StatusFlags | extra_flags
  };

  AccessibilityItem(ImGuiID);

  bool operator<(const AccessibilityItem &o) const { return m_id < o.m_id; }
  bool operator<(const ImGuiID o) const { return m_id < o; }

  ImGuiID id() const { return m_id; }
  ImVec4 rect() const { return m_rect; }
  auto platformData() const { return m_platform; }

  void setRect(const ImVec4 &rect) { m_rect = rect; }
  void setState(const State &); // platform-specific

  void touch();
  bool isOlderThan(const int frame) { return m_lastActiveFrame < frame; }

private:
  ImGuiID m_id;
  int m_lastActiveFrame;
  ImVec4 m_rect;
  std::shared_ptr<void> m_platform;
};

#endif
