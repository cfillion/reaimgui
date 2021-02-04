#include "window.hpp"

#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <imgui/backends/imgui_impl_metal.h>
#include <swell/swell.h>

#include "textinput.mm"

constexpr int SWELL_ENABLED_NO_FOCUS { -1000 };

struct MetalSharedState {
  MetalSharedState();
  ~MetalSharedState();

  id<MTLDevice> device;
  id<MTLCommandQueue> commandQueue;
  MTLRenderPassDescriptor *renderPass;
};

static std::weak_ptr<MetalSharedState> g_shared;

struct Window::PlatformDetails {
  std::shared_ptr<MetalSharedState> shared;
  CAMetalLayer *layer;
  TextInput *textInput;

  // per-frame
  CFAbsoluteTime lastTime {};
  id<MTLCommandBuffer> commandBuffer;
  id<MTLRenderCommandEncoder> renderEncoder;
  id<CAMetalDrawable> drawable;
};

MetalSharedState::MetalSharedState()
{
  device = MTLCreateSystemDefaultDevice();
  assert(device);

  commandQueue = [device newCommandQueue];
  renderPass = [MTLRenderPassDescriptor new];

  ImGui_ImplMetal_Init(device);
}

MetalSharedState::~MetalSharedState()
{
  ImGui_ImplMetal_Shutdown();
}

void Window::platformInit()
{
  assert(!m_p);
  m_p = new PlatformDetails;

  if(g_shared.expired())
    g_shared = m_p->shared = std::make_shared<MetalSharedState>();
  else
    m_p->shared = g_shared.lock();

  m_p->layer = [CAMetalLayer layer];
  m_p->layer.device = m_p->shared->device;
  m_p->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  m_p->layer.opaque = NO;

  // greatly reduces the latency from presentDrawable
  // m_p->layer.maximumDrawableCount = 2;
  // m_p->layer.displaySyncEnabled = false;

  NSView *view { (__bridge NSView *)m_handle };
  view.wantsLayer = YES;
  view.layer = m_p->layer;

  // enable transparency
  [[view window] setOpaque:NO];
  [[view window] setBackgroundColor:[NSColor clearColor]];

  m_p->textInput = [[TextInput alloc] initWithWindow:this];
  [view addSubview:m_p->textInput];
  [[view window] makeFirstResponder:m_p->textInput];

  // prevent the window from overwriting the first responder on focus
  EnableWindow(m_handle, SWELL_ENABLED_NO_FOCUS);
}

void Window::platformTeardown()
{
  delete m_p;
}

void Window::platformBeginFrame()
{
  // returns nil if no drawable is ready after a 1 second timeout
  m_p->drawable = [m_p->layer nextDrawable];

  if(!m_p->drawable)
    return;

  NSView *view { (__bridge NSView *)m_handle };
  m_p->layer.drawableSize = view.bounds.size;

  ImGuiIO &io { ImGui::GetIO() };
  const float dpi { static_cast<float>([view.window backingScaleFactor]) };
  io.DisplaySize = ImVec2(view.bounds.size.width, view.bounds.size.height);
  io.DisplayFramebufferScale = ImVec2(dpi, dpi);

  if(!m_p->lastTime)
    m_p->lastTime = CFAbsoluteTimeGetCurrent();
  const CFAbsoluteTime now { CFAbsoluteTimeGetCurrent() };
  io.DeltaTime = static_cast<float>(now - m_p->lastTime);
  m_p->lastTime = now;

  auto *colorAttachment { m_p->shared->renderPass.colorAttachments[0] };
  colorAttachment.clearColor = std::apply(MTLClearColorMake, m_clearColor);
  colorAttachment.texture = m_p->drawable.texture;
  colorAttachment.loadAction = MTLLoadActionClear;
  colorAttachment.storeAction = MTLStoreActionStore;

  m_p->commandBuffer = [m_p->shared->commandQueue commandBuffer];
  m_p->renderEncoder = [m_p->commandBuffer renderCommandEncoderWithDescriptor:m_p->shared->renderPass];
  [m_p->renderEncoder pushDebugGroup:@"reaper_imgui"];

  ImGui_ImplMetal_NewFrame(m_p->shared->renderPass);
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
    // [m_p->commandBuffer presentDrawable:m_p->drawable];
    [m_p->commandBuffer commit];
    [m_p->drawable present]; // much faster than commandBuffer::presentDrawable
  }

  m_p->drawable = nil; // tell ARC it can release it now
}
