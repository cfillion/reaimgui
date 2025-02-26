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

#include "opengl_renderer.hpp"

#include "error.hpp"
#include "context.hpp"
#include "window.hpp"

#ifdef __APPLE__
#  define GL_SILENCE_DEPRECATION
#  include <OpenGL/gl3.h>
#elif _WIN32
#  include <imgui/backends/imgui_impl_opengl3_loader.h>
constexpr int GL_TEXTURE_WRAP_S {0x2802},
              GL_TEXTURE_WRAP_T {0x2803},
              GL_REPEAT         {0x2901},
              GL_NO_ERROR       {0x0000};
#else
#  include <epoxy/gl.h>
#endif

#include <imgui/imgui.h>

REGISTER_RENDERER(90, opengl3, "OpenGL 3.2",
  OpenGLRenderer::creator, OpenGLRenderer::flags);

constexpr const char *VERTEX_SHADER {R"(
#version 150

uniform mat4 ProjMtx;

in vec2 Position;
in vec2 UV;
in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

void main()
{
  Frag_UV = UV;
  Frag_Color = Color;
  gl_Position = ProjMtx * vec4(Position.xy,0,1);
}
)"};

constexpr const char *FRAGMENT_SHADER {R"(
#version 150

uniform sampler2D Texture;

in vec2 Frag_UV;
in vec4 Frag_Color;

out vec4 Out_Color;

void main()
{
  Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
}
)"};

// these must match with the sizes of the corresponding member arrays
enum Buffers   {VertexBuf, IndexBuf};
enum Textures  {FontTex};
enum Locations {ProjMtxUniLoc, TexUniLoc,
                VtxColorAttrLoc, VtxPosAttrLoc, VtxUVAttrLoc};

OpenGLRenderer::Shared::Shared()
  : m_setupCount {}
{
}

void OpenGLRenderer::Shared::setup()
{
  const unsigned int vertShader {glCreateShader(GL_VERTEX_SHADER)};
  glShaderSource(vertShader, 1, &VERTEX_SHADER, nullptr);
  glCompileShader(vertShader);

  const unsigned int fragShader {glCreateShader(GL_FRAGMENT_SHADER)};
  glShaderSource(fragShader, 1, &FRAGMENT_SHADER, nullptr);
  glCompileShader(fragShader);

  m_program = glCreateProgram();
  glAttachShader(m_program, vertShader);
  glAttachShader(m_program, fragShader);
  glLinkProgram(m_program);

  glDetachShader(m_program, vertShader);
  glDetachShader(m_program, fragShader);
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  int shadersStatus;
  glGetProgramiv(m_program, GL_LINK_STATUS, &shadersStatus);
  if(!shadersStatus)
    throw backend_error {"failed to compile or link OpenGL shaders"};

  m_locations[ProjMtxUniLoc]   = glGetUniformLocation(m_program, "ProjMtx");
  m_locations[TexUniLoc]       = glGetUniformLocation(m_program, "Texture");
  m_locations[VtxColorAttrLoc] = glGetAttribLocation (m_program, "Color");
  m_locations[VtxPosAttrLoc]   = glGetAttribLocation (m_program, "Position");
  m_locations[VtxUVAttrLoc]    = glGetAttribLocation (m_program, "UV");

  glActiveTexture(GL_TEXTURE0);
}

void OpenGLRenderer::Shared::teardown()
{
  glDeleteProgram(m_program);
  glDeleteTextures(m_textures.size(), m_textures.data());
}

void OpenGLRenderer::Shared::textureCommand(const TextureCmd &cmd)
{
  switch(cmd.type) {
  case TextureCmd::Insert:
    m_textures.insert(m_textures.begin() + cmd.offset, cmd.size, 0);
    glGenTextures(cmd.size, m_textures.data() + cmd.offset);
    [[fallthrough]];
  case TextureCmd::Update:
    for(size_t i {}; i < cmd.size; ++i) {
      int width, height;
      const unsigned char *pixels {cmd[i].getPixels(&width, &height)};
      glBindTexture(GL_TEXTURE_2D, m_textures[cmd.offset + i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }
    break;
  case TextureCmd::Remove:
    glDeleteTextures(cmd.size, m_textures.data() + cmd.offset);
    m_textures.erase(m_textures.begin() + cmd.offset,
                     m_textures.begin() + cmd.offset + cmd.size);
    break;
  }
}

OpenGLRenderer::OpenGLRenderer
  (RendererFactory *factory, Window *window, const bool share)
  : Renderer {window}
{
  m_shared = factory->getSharedData<Shared>();
  if(!m_shared || !share) {
    m_shared = std::make_shared<Shared>();
    factory->setSharedData(m_shared);
  }
}

void OpenGLRenderer::setup()
{
  if(++m_shared->m_setupCount == 1)
    m_shared->setup();

  glUseProgram(m_shared->m_program);
  glUniform1i(m_shared->m_locations[TexUniLoc], 0);

  glGenVertexArrays(1, &m_vbo);
  glBindVertexArray(m_vbo);

  glGenBuffers(m_buffers.size(), m_buffers.data());
  glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VertexBuf]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[IndexBuf]);
  glEnableVertexAttribArray(m_shared->m_locations[VtxPosAttrLoc]);
  glVertexAttribPointer(m_shared->m_locations[VtxPosAttrLoc],
    2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
    reinterpret_cast<void *>(offsetof(ImDrawVert, pos)));
  glEnableVertexAttribArray(m_shared->m_locations[VtxUVAttrLoc]);
  glVertexAttribPointer(m_shared->m_locations[VtxUVAttrLoc],
    2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
    reinterpret_cast<void *>(offsetof(ImDrawVert, uv)));
  glEnableVertexAttribArray(m_shared->m_locations[VtxColorAttrLoc]);
  glVertexAttribPointer(m_shared->m_locations[VtxColorAttrLoc],
    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert),
    reinterpret_cast<void *>(offsetof(ImDrawVert, col)));

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLRenderer::teardown()
{
  glDeleteBuffers(m_buffers.size(), m_buffers.data());
  glDeleteVertexArrays(1, &m_vbo);

  if(m_shared->m_setupCount-- == 1)
    m_shared->teardown();
}

void OpenGLRenderer::render(const bool flip)
{
  using namespace std::placeholders;
  m_window->context()->textureManager()->update(&m_shared->m_cookie,
    std::bind(&Shared::textureCommand, m_shared.get(), _1));

  const ImGuiViewport *viewport {m_window->viewport()};
  const ImDrawData *drawData {viewport->DrawData};

  if(!(viewport->Flags & ImGuiViewportFlags_NoRendererClear)) {
    glClearColor(0.f, 0.f, 0.f, 0.f); // premultiplied alpha
    glClear(GL_COLOR_BUFFER_BIT);
  }

  glEnable(GL_SCISSOR_TEST);

  const float height {drawData->DisplaySize.y * viewport->DpiScale};
  glViewport(0, 0, drawData->DisplaySize.x * viewport->DpiScale, height);

  // re-bind non-shared objets (we're reusing the same GL context on Windows)
  glBindVertexArray(m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VertexBuf]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[IndexBuf]);

  // update shader variables
  const ProjMtx projMtx {drawData->DisplayPos, drawData->DisplaySize, flip};
  glUniformMatrix4fv(m_shared->m_locations[ProjMtxUniLoc], 1, GL_FALSE, &projMtx);

  const ImVec2 &clipOffset {drawData->DisplayPos},
               &clipScale  {viewport->DpiScale, viewport->DpiScale};
  for(int i {0}; i < drawData->CmdListsCount; ++i) {
    const ImDrawList *cmdList {drawData->CmdLists[i]};

    glBufferData(GL_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(cmdList->VtxBuffer.Size * sizeof(ImDrawVert)),
      static_cast<const void *>(cmdList->VtxBuffer.Data), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(cmdList->IdxBuffer.Size * sizeof(ImDrawIdx)),
      static_cast<const void *>(cmdList->IdxBuffer.Data), GL_STREAM_DRAW);

    for(int j {0}; j < cmdList->CmdBuffer.Size; ++j) {
      const ImDrawCmd *cmd {&cmdList->CmdBuffer[j]};
      if(cmd->UserCallback)
        continue; // no need to call the callback, not using them

      const ClipRect clipRect {cmd->ClipRect, clipOffset, clipScale};
      if(!clipRect)
        continue;
      glScissor(clipRect.left, flip ? clipRect.top : height - clipRect.bottom,
        clipRect.right - clipRect.left, clipRect.bottom - clipRect.top);

      // Bind texture, Draw
      glBindTexture(GL_TEXTURE_2D, m_shared->m_textures[cmd->GetTexID()]);
      glDrawElementsBaseVertex(GL_TRIANGLES, cmd->ElemCount,
        sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
        (void*)(intptr_t)(cmd->IdxOffset * sizeof(ImDrawIdx)),
        cmd->VtxOffset);
    }
  }

  // allow glClear to modify the whole framebuffer
  glDisable(GL_SCISSOR_TEST);

  const GLenum err {glGetError()};
  if(err != GL_NO_ERROR)
    throw backend_error {"rendering failed with OpenGL error {:#x}", err};
}
