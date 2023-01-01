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

#include "opengl_renderer.hpp"

#include "error.hpp"
#include "fast_set.hpp"
#include "window.hpp"

#include <AppKit/AppKit.h>
#include <imgui/imgui.h>
#include <swell/swell-types.h>

using GLPool = FastSet<NSOpenGLContext *>;

class CocoaOpenGL : public OpenGLRenderer {
public:
  CocoaOpenGL(RendererFactory *, Window *);
  ~CocoaOpenGL();

  void setSize(ImVec2) override;
  void render(void *) override;
  void swapBuffers(void *) override;

private:
  GLPool *contextPool() const;
  NSOpenGLContext *m_gl;
};

class MakeCurrent {
public:
  MakeCurrent(NSOpenGLContext *gl)
    : m_gl { gl }
  {
    [m_gl makeCurrentContext];
  }

  ~MakeCurrent()
  {
    [NSOpenGLContext clearCurrentContext];
  }

private:
  NSOpenGLContext *m_gl;
};

decltype(OpenGLRenderer::creator) OpenGLRenderer::creator
  { &Renderer::create<CocoaOpenGL> };

CocoaOpenGL::CocoaOpenGL(RendererFactory *factory, Window *window)
  : OpenGLRenderer { factory, window }
{
  NSView *view { (__bridge NSView *)window->nativeHandle() };
  [view setWantsBestResolutionOpenGLSurface:YES]; // retina
  [view setWantsLayer:YES]; // required to be transparent before resizing

  constexpr NSOpenGLPixelFormatAttribute attrs[] {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFADoubleBuffer,
    kCGLPFASupportsAutomaticGraphicsSwitching,
    0
  };

  if(!m_shared->m_platform)
    m_shared->m_platform = std::make_shared<GLPool>();

  auto pool { contextPool() };
  NSOpenGLPixelFormat *fmt { [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] };
  NSOpenGLContext *share { pool->empty() ? nil : pool->front() };
  m_gl = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:share];
  if(!m_gl)
    throw backend_error { "failed to initialize OpenGL 3.2 core context" };

  pool->insert(m_gl);

  GLint value { 0 }; // enable transparency
  [m_gl setValues:&value forParameter:NSOpenGLContextParameterSurfaceOpacity];

  MakeCurrent cur { m_gl };
  setup();
}

CocoaOpenGL::~CocoaOpenGL()
{
  MakeCurrent cur { m_gl };
  teardown();
  contextPool()->erase(m_gl);
}

GLPool *CocoaOpenGL::contextPool() const
{
  return std::static_pointer_cast<GLPool>(m_shared->m_platform).get();
}

void CocoaOpenGL::setSize(ImVec2)
{
  [m_gl update];
}

void CocoaOpenGL::render(void *)
{
  // the intial setView in show() may fail if the view doesn't have a "device"
  // (eg. when docked not activated = hidden NSView)
  if(![m_gl view])
    [m_gl setView:(__bridge NSView *)m_window->nativeHandle()];

  MakeCurrent cur { m_gl };
  OpenGLRenderer::updateTextures();
  OpenGLRenderer::render(false);
}

void CocoaOpenGL::swapBuffers(void *)
{
  [m_gl flushBuffer];
}
