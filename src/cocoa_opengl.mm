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

#define GL_SILENCE_DEPRECATION
#include <AppKit/AppKit.h>
#include <QuartzCore/CATransaction.h>
#include <imgui/imgui.h>
#include <swell/swell-types.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

class CocoaOpenGL;

@interface OpenGLLayer : NSOpenGLLayer {
@private
  NSOpenGLContext *m_oldGlCtx;
  CocoaOpenGL *m_renderer;
  bool m_inRender, m_didInit, m_forceSoftware;
}

- (instancetype)initWithRenderer:(CocoaOpenGL *)renderer forceSoftware:(bool)f;
- (void)setContentsScale:(CGFloat)scale;
- (void)render;

- (NSOpenGLPixelFormat *)openGLPixelFormatForDisplayMask:(uint32_t)mask;

- (NSOpenGLContext *)openGLContextForPixelFormat:(NSOpenGLPixelFormat *)pixelFormat;
- (void)releaseCGLContext:(CGLContextObj)ctx;

- (BOOL)canDrawInOpenGLContext:(NSOpenGLContext *)gl
                   pixelFormat:(CGLPixelFormatObj)pf
                  forLayerTime:(CFTimeInterval)t
                   displayTime:(const CVTimeStamp *)ts;
- (void)drawInOpenGLContext:(NSOpenGLContext *)gl
                pixelFormat:(CGLPixelFormatObj)pf
               forLayerTime:(CFTimeInterval)t
                displayTime:(const CVTimeStamp *)ts;
@end

class CocoaOpenGL final : public OpenGLRenderer {
public:
  CocoaOpenGL(RendererFactory *, Window *);
  ~CocoaOpenGL();

  void setSize(ImVec2) override;
  void render(void *) override;
  void swapBuffers(void *) override;

  // protected friend OpenGLLayer
  using OpenGLRenderer::setup;
  using OpenGLRenderer::render;
  using OpenGLRenderer::teardown;
  using OpenGLRenderer::m_shared;

private:
  OpenGLLayer *m_layer;
};

struct CocoaOpenGLShared {
  CocoaOpenGLShared() : ctx {nil} {}
  NSOpenGLContext *ctx;
};

class MakeCurrent {
public:
  MakeCurrent(NSOpenGLContext *gl)
    : m_gl {gl}
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
  {&Renderer::create<CocoaOpenGL>};
decltype(OpenGLRenderer::flags) OpenGLRenderer::flags
  {RendererType::Available | RendererType::CanForceSoftware};

CocoaOpenGL::CocoaOpenGL(RendererFactory *factory, Window *window)
  : OpenGLRenderer {factory, window}
{
  if(!m_shared->m_platform)
    m_shared->m_platform = std::make_shared<CocoaOpenGLShared>();

  m_layer = [[OpenGLLayer alloc] initWithRenderer:this
                                    forceSoftware:factory->wantSoftware()];

  HWND hwnd {m_window->nativeHandle()};
  SetOpaque(hwnd, false);
  NSView *view {(__bridge NSView *)hwnd};
  [view setWantsBestResolutionOpenGLSurface:YES]; // enable HiDPI support
  [view setLayer:m_layer];
  [view setWantsLayer:YES];
  // don't stretch when resizing & native decorations are enabled
  [view setLayerContentsPlacement:NSViewLayerContentsPlacementTopLeft];
}

CocoaOpenGL::~CocoaOpenGL()
{
  if(NSOpenGLContext *gl {[m_layer openGLContext]}) {
    MakeCurrent cur {gl};
    teardown();
  }

  // Prevent m_layer from using m_renderer (this) after free
  // in openGLContextForPixelFormat when an error occurs in another window's
  // render() before it had a chance to initialize.
  [[m_layer view] setLayer:nil];
}

void CocoaOpenGL::setSize(ImVec2)
{
}

void CocoaOpenGL::render(void *)
{
  [m_layer render];
}

void CocoaOpenGL::swapBuffers(void *)
{
}

@implementation OpenGLLayer
- (instancetype)initWithRenderer:(CocoaOpenGL *)renderer
                   forceSoftware:(bool)forceSoftware
{
  self = [super init];
  [self setOpaque:NO];
  [self setContentsGravity:kCAGravityBottomLeft]; // don't stretch when resizing
  m_oldGlCtx = nil;
  m_renderer = renderer;
  m_inRender = m_didInit = false;
  m_forceSoftware = forceSoftware;
  return self;
}

- (void)setContentsScale:(CGFloat)scale
{
  // disable animation when scale changes
  [CATransaction begin];
  [CATransaction setDisableActions:YES];
  [super setContentsScale:scale];
  [CATransaction commit];
}

- (void)render
{
  m_inRender = true;
  [super setNeedsDisplay];
  [super displayIfNeeded];
  m_inRender = false;

  if(m_didInit && ![self openGLContext])
    throw backend_error {"failed to initialize a OpenGL 3.2 context"};
}

- (NSOpenGLPixelFormat *)openGLPixelFormatForDisplayMask:(uint32_t)mask
{
  size_t attrc {};
  NSOpenGLPixelFormatAttribute attrs[16];
  attrs[attrc++] = NSOpenGLPFAOpenGLProfile;
  attrs[attrc++] = NSOpenGLProfileVersion3_2Core;

  attrs[attrc++] = NSOpenGLPFAScreenMask; attrs[attrc++] = mask;
  attrs[attrc++] = kCGLPFASupportsAutomaticGraphicsSwitching;
  attrs[attrc++] = NSOpenGLPFADoubleBuffer;

  attrs[attrc++] = NSOpenGLPFAClosestPolicy;
  attrs[attrc++] = NSOpenGLPFAColorSize; attrs[attrc++] = 24;
  attrs[attrc++] = NSOpenGLPFAAlphaSize; attrs[attrc++] = 8;

  if(m_forceSoftware) {
    attrs[attrc++] = NSOpenGLPFARendererID;
    attrs[attrc++] = kCGLRendererGenericFloatID; // Apple Software Renderer
  }

  attrs[attrc++] = 0;
  assert(attrc < std::size(attrs));


  // OK to report lack of OpenGL context as initialization failure after this point
  m_didInit = true;

  return [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
}

- (NSOpenGLContext *)openGLContextForPixelFormat:(NSOpenGLPixelFormat *)fmt
{
  auto globalShared {m_renderer->m_shared};

  CocoaOpenGLShared *shared
    {std::static_pointer_cast<CocoaOpenGLShared>(globalShared->m_platform).get()};
  NSOpenGLContext *gl
    {[[NSOpenGLContext alloc] initWithFormat:fmt shareContext:shared->ctx]};

  if(m_oldGlCtx) {
    MakeCurrent cur {m_oldGlCtx};

    // increases the reference count so that teardown() below doesn't incorrectly
    // frees shared OpenGL resources and setup() doesn't re-initializes it
    if(gl)
      ++globalShared->m_setupCount;
    else
      m_oldGlCtx = nil;

    m_renderer->teardown();
  }

  if(!gl)
    return nil; // CocoaOpenGl::render(void*) will report the situation
  if(!shared->ctx)
    shared->ctx = gl;

  MakeCurrent cur {gl};
  m_renderer->setup();

  if(m_oldGlCtx) {
    --globalShared->m_setupCount;
    m_oldGlCtx = nil;
  }

  return gl;
}

- (void)releaseCGLContext:(CGLContextObj)gl
{
  // This is invoked before the new context is created when moving between monitors
  // Because of that we can't do teardown() here, as that would free shared resources
  //
  // Allocating a new one because [[self openGLContext] CGLContextObj] != gl on 10.9
  m_oldGlCtx = [[NSOpenGLContext alloc] initWithCGLContextObj:gl];
  [super releaseCGLContext:gl];
}

- (BOOL)canDrawInOpenGLContext:(NSOpenGLContext *)gl
                   pixelFormat:(CGLPixelFormatObj)pf
                  forLayerTime:(CFTimeInterval)t
                   displayTime:(const CVTimeStamp *)ts
{
  return m_inRender;
}

- (void)drawInOpenGLContext:(NSOpenGLContext *)gl
                pixelFormat:(CGLPixelFormatObj)pf
               forLayerTime:(CFTimeInterval)t
                displayTime:(const CVTimeStamp *)ts
{
  MakeCurrent cur {gl};
  m_renderer->render(false);
}
@end
