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

#include "opengl_renderer.hpp"

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#elif _WIN32
#  include <GL/gl3w.h>
#else
#  include <epoxy/gl.h>
#endif

#include <imgui/imgui.h>

constexpr const char *VERTEX_SHADER { R"(
#version 140

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
)" };

constexpr const char *FRAGMENT_SHADER { R"(
#version 140

uniform sampler2D Texture;

in vec2 Frag_UV;
in vec4 Frag_Color;

out vec4 Out_Color;

void main()
{
  Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
}
)" };

// these must match with the sizes of the corresponding member arrays
enum Buffers   { VertexBuf, IndexBuf };
enum Textures  { FontTex };
enum Locations { ProjMtxUniLoc, TexUniLoc,
                 VtxColorAttrLoc, VtxPosAttrLoc, VtxUVAttrLoc };

void OpenGLRenderer::Shared::setup()
{
  unsigned int vertShader { glCreateShader(GL_VERTEX_SHADER) };
  glShaderSource(vertShader, 1, &VERTEX_SHADER, nullptr);
  glCompileShader(vertShader);

  unsigned int fragShader { glCreateShader(GL_FRAGMENT_SHADER) };
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

  m_locations[ProjMtxUniLoc]   = glGetUniformLocation(m_program, "ProjMtx");
  m_locations[TexUniLoc]       = glGetUniformLocation(m_program, "Texture");
  m_locations[VtxColorAttrLoc] = glGetAttribLocation(m_program,  "Color");
  m_locations[VtxPosAttrLoc]   = glGetAttribLocation(m_program,  "Position");
  m_locations[VtxUVAttrLoc]    = glGetAttribLocation(m_program,  "UV");
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
      unsigned char *pixels { cmd.manager->getPixels(cmd.offset + i, &width, &height) };
      glBindTexture(GL_TEXTURE_2D, m_textures[cmd.offset + i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

OpenGLRenderer::OpenGLRenderer(RendererFactory *factory, const bool share)
{
  m_shared = factory->getSharedData<Shared>();
  if(!m_shared || !share) {
    m_shared = std::make_shared<Shared>();
    factory->setSharedData(m_shared);
  }
}

void OpenGLRenderer::setup()
{
  if(m_shared.use_count() == 1)
    m_shared->setup();

  glUseProgram(m_shared->m_program);

  glGenVertexArrays(1, &m_vbo);
  glBindVertexArray(m_vbo);

  glGenBuffers(m_buffers.size(), m_buffers.data());
  glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VertexBuf]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[IndexBuf]);
  glEnableVertexAttribArray(m_shared->m_locations[VtxPosAttrLoc]);
  glVertexAttribPointer(m_shared->m_locations[VtxPosAttrLoc],
    2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
    reinterpret_cast<void *>(IM_OFFSETOF(ImDrawVert, pos)));
  glEnableVertexAttribArray(m_shared->m_locations[VtxUVAttrLoc]);
  glVertexAttribPointer(m_shared->m_locations[VtxUVAttrLoc],
    2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
    reinterpret_cast<void *>(IM_OFFSETOF(ImDrawVert, uv)));
  glEnableVertexAttribArray(m_shared->m_locations[VtxColorAttrLoc]);
  glVertexAttribPointer(m_shared->m_locations[VtxColorAttrLoc],
    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert),
    reinterpret_cast<void *>(IM_OFFSETOF(ImDrawVert, col)));

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  GLint major {}, minor {};
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);

  ImGuiIO &io { ImGui::GetIO() };
  if(major < 3 || minor < 1) {
    // this is the only version check on Linux
    char msg[512];
    snprintf(msg, sizeof(msg),
      "OpenGL v3.1 or newer required, got v%d.%d", major, minor);
    throw backend_error { msg };
  }
  else if(major > 3 || minor >= 2) // enable glDrawElementsBaseVertex
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
}

void OpenGLRenderer::teardown()
{
  if(m_shared.use_count() == 1)
    m_shared->teardown();

  glDeleteBuffers(m_buffers.size(), m_buffers.data());
  glDeleteVertexArrays(1, &m_vbo);
}

void OpenGLRenderer::updateTextures(const TextureManager *manager)
{
  using namespace std::placeholders;
  manager->update(&m_shared->m_cookie,
    std::bind(&Shared::textureCommand, m_shared.get(), _1));
}

void OpenGLRenderer::render(ImGuiViewport *viewport, const bool flip)
{
  ImDrawData *drawData { viewport->DrawData };

  // this is not done by Dear ImGui at this moment, but might be in the future
  drawData->FramebufferScale = { viewport->DpiScale, viewport->DpiScale };

  const int fbWidth  { static_cast<int>(drawData->DisplaySize.x * drawData->FramebufferScale.x) },
            fbHeight { static_cast<int>(drawData->DisplaySize.y * drawData->FramebufferScale.y) };

  if(fbWidth <= 0 || fbHeight <= 0)
    return;

  if(!(viewport->Flags & ImGuiViewportFlags_NoRendererClear)) {
    glClearColor(0.f, 0.f, 0.f, 0.f); // premultiplied alpha
    glClear(GL_COLOR_BUFFER_BIT);
  }

  glEnable(GL_SCISSOR_TEST);

  glViewport(0, 0, fbWidth, fbHeight);

  // re-bind non-shared objets (we're reusing the same GL context on Windows)
  glBindVertexArray(m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VertexBuf]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[IndexBuf]);

  float L { drawData->DisplayPos.x },
        R { drawData->DisplayPos.x + drawData->DisplaySize.x },
        T { drawData->DisplayPos.y },
        B { drawData->DisplayPos.y + drawData->DisplaySize.y };

  if(flip)
    std::swap(T, B);

  const float orthoProjection[4][4] {
    { 2.f/(R-L),   0.f,         0.f, 0.f },
    { 0.f,         2.f/(T-B),   0.f, 0.f },
    { 0.f,         0.f,        -1.f, 0.f },
    { (R+L)/(L-R), (T+B)/(B-T), 0.f, 1.f },
  };

  // update shader variables
  glUniform1i(m_shared->m_locations[TexUniLoc], 0);
  // glActiveTexture(GL_TEXTURE0);
  glUniformMatrix4fv(m_shared->m_locations[ProjMtxUniLoc],
    1, GL_FALSE, &orthoProjection[0][0]);

  const ImVec2 &clipOffset { drawData->DisplayPos },
               &clipScale  { drawData->FramebufferScale };

  for(int i = 0; i < drawData->CmdListsCount; ++i) {
    const ImDrawList *cmdList { drawData->CmdLists[i] };
    const bool useVertexOffset
      { (cmdList->Flags & ImDrawListFlags_AllowVtxOffset) != 0 };

    glBufferData(GL_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(cmdList->VtxBuffer.Size * sizeof(ImDrawVert)),
      static_cast<const GLvoid *>(cmdList->VtxBuffer.Data), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(cmdList->IdxBuffer.Size * sizeof(ImDrawIdx)),
      static_cast<const GLvoid *>(cmdList->IdxBuffer.Data), GL_STREAM_DRAW);

    for(int j = 0; j < cmdList->CmdBuffer.Size; ++j) {
      const ImDrawCmd *cmd { &cmdList->CmdBuffer[j] };
      if(cmd->UserCallback)
        continue; // no need to call the callback, not using them

      // Project scissor/clipping rectangles into framebuffer space
      const ImVec4 clipRect {
        (cmd->ClipRect.x - clipOffset.x) * clipScale.x,
        (cmd->ClipRect.y - clipOffset.y) * clipScale.y,
        (cmd->ClipRect.z - clipOffset.x) * clipScale.x,
        (cmd->ClipRect.w - clipOffset.y) * clipScale.y,
      };

      if(clipRect.x >= fbWidth || clipRect.y >= fbHeight ||
          clipRect.z < 0.0f || clipRect.w < 0.0f)
        continue;

      // Apply scissor/clipping rectangle
      glScissor(clipRect.x, flip ? clipRect.y : fbHeight - clipRect.w,
        clipRect.z - clipRect.x, clipRect.w - clipRect.y);

      // Bind texture, Draw
      glBindTexture(GL_TEXTURE_2D, m_shared->m_textures[cmd->GetTexID()]);
      if(useVertexOffset) {
        glDrawElementsBaseVertex(GL_TRIANGLES, cmd->ElemCount,
          sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
          (void*)(intptr_t)(cmd->IdxOffset * sizeof(ImDrawIdx)),
          cmd->VtxOffset);
      }
      else {
        glDrawElements(GL_TRIANGLES, cmd->ElemCount,
          sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
          (void*)(intptr_t)(cmd->IdxOffset * sizeof(ImDrawIdx)));
      }
    }
  }

  // allow glClear to modify the whole framebuffer
  glDisable(GL_SCISSOR_TEST);
}
