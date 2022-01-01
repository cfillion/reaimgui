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

#ifndef REAIMGUI_OPENGL_RENDERER_HPP
#define REAIMGUI_OPENGL_RENDERER_HPP

#include <array>

struct ImDrawData;
struct ImGuiViewport;

class OpenGLRenderer {
public:
  // minimum OpenGL version
  static constexpr int MIN_MAJOR { 3 };
  static constexpr int MIN_MINOR { 2 };

  static void install();

  OpenGLRenderer();
  ~OpenGLRenderer();

  void uploadFontTex();
  void render(ImGuiViewport *, bool flip = false);

private:
  void initShaders();

  unsigned int m_vbo, m_program;
  std::array<unsigned int, 2> m_buffers;
  std::array<unsigned int, 1> m_textures; // make this a vector for image support?
  std::array<unsigned int, 5> m_locations;
};

#endif
