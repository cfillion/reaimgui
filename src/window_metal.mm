#include "window.hpp"

#include "context_metal.hpp"

#include <tuple>

#include <AppKit/AppKit.h>
#include <QuartzCore/QuartzCore.h>

#include <imgui/backends/imgui_impl_metal.h>
#include <imgui/backends/imgui_impl_osx.h>

struct Window::Impl {
  CAMetalLayer *layer;
  id<MTLCommandBuffer> commandBuffer;
  id<MTLRenderCommandEncoder> renderEncoder;
  id<CAMetalDrawable> drawable;
};

void Window::platformInit()
{
  m_p = new Impl;

  Context_Metal *context { dynamic_cast<Context_Metal *>(m_context.get()) };

  NSView *view { (__bridge NSView *)m_handle };
  NSWindow *nativeWindow { [view window] };

  m_p->layer = [CAMetalLayer layer];
  m_p->layer.maximumDrawableCount = 2;
  m_p->layer.device = context->device();
  m_p->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  m_p->layer.opaque = NO;

  nativeWindow.contentView.layer = m_p->layer;
  nativeWindow.contentView.wantsLayer = YES;
  [nativeWindow setOpaque:NO];
  [nativeWindow setBackgroundColor:[NSColor clearColor]];
}

void Window::platformTeardown()
{
  delete m_p;
}

void Window::platformBeginFrame()
{
  NSView *view { (__bridge NSView *)m_handle };
  m_p->layer.drawableSize = view.frame.size;

  m_p->drawable = [m_p->layer nextDrawable];

  if(!m_p->drawable)
    return;

  Context_Metal *context { dynamic_cast<Context_Metal *>(m_context.get()) };
  m_p->commandBuffer = [context->commandQueue() commandBuffer];

  MTLRenderPassDescriptor *renderPass { context->renderPass() };
  renderPass.colorAttachments[0].clearColor = std::apply(MTLClearColorMake,
  m_clearColor);
  renderPass.colorAttachments[0].texture = m_p->drawable.texture;
  renderPass.colorAttachments[0].loadAction = MTLLoadActionClear;
  renderPass.colorAttachments[0].storeAction = MTLStoreActionStore;

  m_p->renderEncoder = [m_p->commandBuffer renderCommandEncoderWithDescriptor:renderPass];
  [m_p->renderEncoder pushDebugGroup:@"ReaImGui"];

  ImGui_ImplMetal_NewFrame(renderPass);
  ImGui_ImplOSX_NewFrame(view);
}

void Window::platformEndFrame(ImDrawData *drawData)
{
  if(!m_p->drawable)
    return;

  if(drawData)
    ImGui_ImplMetal_RenderDrawData(drawData, m_p->commandBuffer, m_p->renderEncoder);

  [m_p->renderEncoder popDebugGroup];
  [m_p->renderEncoder endEncoding];

  if(drawData) {
    [m_p->commandBuffer presentDrawable:m_p->drawable];
    [m_p->commandBuffer commit];
  }
}
