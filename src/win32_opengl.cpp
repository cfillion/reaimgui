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
#include "window.hpp"

#define IMGL3W_IMPL
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3_loader.h>

// https://registry.khronos.org/OpenGL/api/GL/wglext.h
constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB    {0x2091},
              WGL_CONTEXT_MINOR_VERSION_ARB    {0x2092},
              WGL_CONTEXT_PROFILE_MASK_ARB     {0x9126},
              WGL_CONTEXT_CORE_PROFILE_BIT_ARB {0x0001};
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)
  (HDC hDC, HGLRC hShareContext, const int *attribList);

class Win32OpenGL final : public OpenGLRenderer {
public:
  Win32OpenGL(RendererFactory *, Window *);
  ~Win32OpenGL();

  void setSize(ImVec2) override {}
  void render(void *) override;
  void swapBuffers(void *) override;

private:
  void setPixelFormat();
  void createContext();

  HDC m_dc;
  HGLRC m_gl;
};

class MakeCurrent {
public:
  MakeCurrent(HDC dc, HGLRC gl)
    : m_gl {gl}
  {
    wglMakeCurrent(dc, m_gl);
  }

  ~MakeCurrent()
  {
    wglMakeCurrent(nullptr, nullptr);
  }

private:
  HGLRC m_gl;
};

struct GLDeleter {
  void operator()(HGLRC gl)
  {
    wglDeleteContext(gl);
  }
};

decltype(OpenGLRenderer::creator) OpenGLRenderer::creator
  {&Renderer::create<Win32OpenGL>};
decltype(RendererType::flags) OpenGLRenderer::flags {RendererType::Available};

Win32OpenGL::Win32OpenGL(RendererFactory *factory, Window *window)
  : OpenGLRenderer(factory, window), m_dc {GetDC(window->nativeHandle())}
{
  setPixelFormat();

  if(m_shared->m_platform) {
    using GL = std::remove_pointer_t<HGLRC>;
    m_gl = std::static_pointer_cast<GL>(m_shared->m_platform).get();
  }
  else {
    createContext();
    m_shared->m_platform = {m_gl, GLDeleter {}};
  }

  MakeCurrent cur {m_dc, m_gl};
  setup();
}

Win32OpenGL::~Win32OpenGL()
{
  MakeCurrent cur {m_dc, m_gl};
  teardown();
}

void Win32OpenGL::setPixelFormat()
{
  constexpr PIXELFORMATDESCRIPTOR pfd {
    .nSize      = sizeof(pfd),
    .nVersion   = 1,
    .dwFlags    = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 32,
    .cRedBits   = 8, .cGreenBits = 8, .cBlueBits = 8, .cAlphaBits = 8,
  };

  if(!SetPixelFormat(m_dc, ChoosePixelFormat(m_dc, &pfd), &pfd))
    throw backend_error {"failed to set a suitable pixel format"};
}

void Win32OpenGL::createContext()
{
  HGLRC dummyGl {wglCreateContext(m_dc)}; // creates a legacy (< 2.1) context
  wglMakeCurrent(m_dc, m_gl = dummyGl);

  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB
    {reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>
     (wglGetProcAddress("wglCreateContextAttribsARB"))};

  if(wglCreateContextAttribsARB) {
    // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
    constexpr int attrs[] {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 2,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };

    if(HGLRC coreGl {wglCreateContextAttribsARB(m_dc, nullptr, attrs)}) {
      wglMakeCurrent(m_dc, m_gl = coreGl);
      wglDeleteContext(dummyGl);
    }
  }

  if(imgl3wInit()) {
    wglDeleteContext(m_gl);
    throw backend_error {"OpenGL 3.2 is not available on this system"};
  }
}

void Win32OpenGL::render(void *)
{
  MakeCurrent cur {m_dc, m_gl};
  OpenGLRenderer::render(false);
}

void Win32OpenGL::swapBuffers(void *)
{
  SwapBuffers(m_dc);
}
