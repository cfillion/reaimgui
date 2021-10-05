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

#ifndef REAIMGUI_FLAGS_HPP
#define REAIMGUI_FLAGS_HPP

#include <imgui/imgui_internal.h> // ImHashStr, internal ImGuiInputTextFlags

template<typename T>
class Flags {
public:
  operator T() const { return m_flags; }
  T operator |=(T op) { return m_flags |= op; }
  T operator &=(T op) { return m_flags &= op; }
  T operator &(T op) const { return m_flags & op; }

protected:
  Flags(int *flags) : m_flags { valueOr(flags, 0) } {}

private:
  T m_flags;
};

class InputTextFlags : public Flags<ImGuiInputTextFlags> {
public:
  InputTextFlags(int *flags) : Flags(flags)
  {
    *this &= ~(
      // don't expose these to users
      ImGuiInputTextFlags_CallbackCompletion |
      ImGuiInputTextFlags_CallbackHistory    |
      ImGuiInputTextFlags_CallbackAlways     |
      ImGuiInputTextFlags_CallbackCharFilter |
      ImGuiInputTextFlags_CallbackEdit       |
      ImGuiInputTextFlags_CallbackResize     |

      // reserved for ImGui's internal use
      ImGuiInputTextFlags_Multiline | ImGuiInputTextFlags_NoMarkEdited
    );
  }
};

class SliderFlags : public Flags<ImGuiSliderFlags> {
public:
  SliderFlags(int *flags) : Flags(flags)
  {
    // dear imgui will assert if these bits are set
    *this &= ~ImGuiSliderFlags_InvalidMask_;
  }
};

enum {
  // NoBringToFrontOnFocus isn't exposed in ReaImGui
  ReaImGuiWindowFlags_TopMost = ImGuiWindowFlags_NoBringToFrontOnFocus,
};

class WindowFlags : public Flags<ImGuiWindowFlags> {
public:
  WindowFlags(int *flags) : Flags(flags)
  {
    if(*this & ReaImGuiWindowFlags_TopMost) {
      ImGuiWindowClass topmost;
      topmost.ClassId = ImHashStr("TopMost");
      topmost.ViewportFlagsOverrideSet = ImGuiViewportFlags_TopMost;
      ImGui::SetNextWindowClass(&topmost);
      *this &= ~ReaImGuiWindowFlags_TopMost; // unset NoBringToFrontOnFocus
    }
    else {
      static ImGuiWindowClass normal;
      ImGui::SetNextWindowClass(&normal);
    }
  }
};

#endif
