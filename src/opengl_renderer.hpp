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

#ifndef REAIMGUI_OPENGL_RENDERER_HPP
#define REAIMGUI_OPENGL_RENDERER_HPP

#include "renderer.hpp"
#include "texture.hpp"

#include <array>

class OpenGLRenderer : public Renderer {
public:
  static std::unique_ptr<Renderer>(*creator)(RendererFactory *, Window *);
  static decltype(RendererType::flags) flags;

  OpenGLRenderer(RendererFactory *, Window *, bool share = true);

  using Renderer::render;

protected:
  void render(bool flip);

  struct Shared {
    Shared();
    void setup();
    void teardown();
    void textureCommand(const TextureCmd &);

    unsigned int m_setupCount;
    unsigned int m_program;
    TextureCookie m_cookie;
    std::vector<unsigned int> m_textures;
    std::array<unsigned int, 5> m_locations;
    std::shared_ptr<void> m_platform;
  };

  void setup();
  void teardown();

  std::shared_ptr<Shared> m_shared;

private:
  unsigned int m_vbo;
  std::array<unsigned int, 2> m_buffers;
};

#endif
