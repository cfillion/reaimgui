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

#ifndef REAIMGUI_COCOA_WINDOW_HPP
#define REAIMGUI_COCOA_WINDOW_HPP

#include "window.hpp"

@class EventHandler;
@class InputView;
@class NSView;

using ImGuiViewportFlags = int;

class CocoaWindow final : public Window {
public:
  CocoaWindow(ImGuiViewport *, DockerHost *);
  ~CocoaWindow() override;

  void create() override;
  void show() override;
  void setPosition(ImVec2) override;
  void setSize(ImVec2) override;
  void setFocus() override;
  bool hasFocus() const override;
  void setTitle(const char *) override;
  void setAlpha(float) override;
  void update() override;
  float scaleFactor() const override;
  void setIME(ImGuiPlatformImeData *) override;

  std::optional<LRESULT> handleMessage
    (const unsigned int msg, WPARAM wParam, LPARAM) override;
  int handleAccelerator(MSG *) override;

private:
  NSView *m_view;
  InputView *m_inputView;
  EventHandler *m_eventHandler;
  unsigned int m_defaultStyleMask;
  float m_previousScale;
  ImGuiViewportFlags m_previousFlags;
};

#endif

// vi: ft=objcpp
