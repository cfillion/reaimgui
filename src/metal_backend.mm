#include "backend.hpp"

#define CreateEvent SWELL_CreateEvent
#include "context.hpp"
#include "inputview.hpp"
#undef CreateEvent

#include <Carbon/Carbon.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <imgui/backends/imgui_impl_metal.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

struct MetalSharedState {
  MetalSharedState();
  ~MetalSharedState();

  id<MTLDevice> device;
  id<MTLCommandQueue> commandQueue;
  MTLRenderPassDescriptor *renderPass;
};

static std::weak_ptr<MetalSharedState> g_shared;

class MetalBackend : public Backend {
public:
  MetalBackend(Context *);
  ~MetalBackend() override;

  void beginFrame() override;
  void endFrame(ImDrawData *) override;
  float deltaTime() override;
  float scaleFactor() const override;
  void translateAccel(MSG *) override;

private:
  std::shared_ptr<MetalSharedState> m_shared;

  NSView *m_view;
  CAMetalLayer *m_layer;
  InputView *m_inputView;

  // per-frame
  CFAbsoluteTime m_lastFrame;
  id<MTLCommandBuffer> m_commandBuffer;
  id<MTLRenderCommandEncoder> m_renderEncoder;
  id<CAMetalDrawable> m_drawable;
};

std::unique_ptr<Backend> Backend::create(Context *ctx)
{
  return std::make_unique<MetalBackend>(ctx);
}

MetalSharedState::MetalSharedState()
{
  device = MTLCreateSystemDefaultDevice();
  assert(device);

  commandQueue = [device newCommandQueue];
  renderPass = [MTLRenderPassDescriptor new];

  ImGui_ImplMetal_Init(device);

  // Temprarily enable repeat character input
  // WARNING: this is application-wide!
  NSUserDefaults *defaults { [NSUserDefaults standardUserDefaults] };
  [defaults registerDefaults:@{@"ApplePressAndHoldEnabled":@NO}];
}

MetalSharedState::~MetalSharedState()
{
  ImGui_ImplMetal_Shutdown();
}

MetalBackend::MetalBackend(Context *ctx)
  : m_view { (__bridge NSView *)ctx->handle() }, m_lastFrame {}
{
  if(g_shared.expired())
    g_shared = m_shared = std::make_shared<MetalSharedState>();
  else
    m_shared = g_shared.lock();

  m_layer = [CAMetalLayer layer];
  m_layer.device = m_shared->device;
  m_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  m_layer.opaque = NO;

  // greatly reduces the latency from presentDrawable
  // m_layer.maximumDrawableCount = 2;
  // m_layer.displaySyncEnabled = false;

  m_view.wantsLayer = YES;
  m_view.layer = m_layer;

  // enable transparency
  // [[view window] setOpaque:NO];
  // [[view window] setBackgroundColor:[NSColor clearColor]];

  m_inputView = [[InputView alloc] initWithContext:ctx parent:m_view];

  ImGuiIO &io { ImGui::GetIO() };
  io.KeyMap[ImGuiKey_Tab]         = kVK_Tab;
  io.KeyMap[ImGuiKey_LeftArrow]   = kVK_LeftArrow;
  io.KeyMap[ImGuiKey_RightArrow]  = kVK_RightArrow;
  io.KeyMap[ImGuiKey_UpArrow]     = kVK_UpArrow;
  io.KeyMap[ImGuiKey_DownArrow]   = kVK_DownArrow;
  io.KeyMap[ImGuiKey_PageUp]      = kVK_PageUp;
  io.KeyMap[ImGuiKey_PageDown]    = kVK_PageDown;
  io.KeyMap[ImGuiKey_Home]        = kVK_Home;
  io.KeyMap[ImGuiKey_End]         = kVK_End;
  io.KeyMap[ImGuiKey_Insert]      = kVK_Help;
  io.KeyMap[ImGuiKey_Delete]      = kVK_ForwardDelete;
  io.KeyMap[ImGuiKey_Backspace]   = kVK_Delete;
  io.KeyMap[ImGuiKey_Space]       = kVK_Space;
  io.KeyMap[ImGuiKey_Enter]       = kVK_Return;
  io.KeyMap[ImGuiKey_Escape]      = kVK_Escape;
  io.KeyMap[ImGuiKey_KeyPadEnter] = kVK_Return;
  io.KeyMap[ImGuiKey_A]           = kVK_ANSI_A;
  io.KeyMap[ImGuiKey_C]           = kVK_ANSI_C;
  io.KeyMap[ImGuiKey_V]           = kVK_ANSI_V;
  io.KeyMap[ImGuiKey_X]           = kVK_ANSI_X;
  io.KeyMap[ImGuiKey_Y]           = kVK_ANSI_Y;
  io.KeyMap[ImGuiKey_Z]           = kVK_ANSI_Z;
}

MetalBackend::~MetalBackend()
{
}

void MetalBackend::beginFrame()
{
  // returns nil if no drawable is ready after a 1 second timeout
  m_drawable = [m_layer nextDrawable];

  if(!m_drawable)
    return;

  m_layer.drawableSize = m_view.bounds.size;

  auto *colorAttachment { m_shared->renderPass.colorAttachments[0] };
  // colorAttachment.clearColor = std::apply(MTLClearColorMake, m_clearColor);
  colorAttachment.texture = m_drawable.texture;
  colorAttachment.loadAction = MTLLoadActionClear;
  colorAttachment.storeAction = MTLStoreActionStore;

  m_commandBuffer = [m_shared->commandQueue commandBuffer];
  m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:m_shared->renderPass];
  [m_renderEncoder pushDebugGroup:@"reaper_imgui"];

  ImGui_ImplMetal_NewFrame(m_shared->renderPass);
}

void MetalBackend::endFrame(ImDrawData *drawData)
{
  if(!m_drawable)
    return;

  if(drawData)
    ImGui_ImplMetal_RenderDrawData(drawData, m_commandBuffer, m_renderEncoder);

  [m_renderEncoder popDebugGroup];
  [m_renderEncoder endEncoding];

  if(drawData) {
    // [m_commandBuffer presentDrawable:m_drawable];
    [m_commandBuffer commit];
    [m_drawable present]; // much faster than commandBuffer::presentDrawable
  }

  m_drawable = nil; // tell ARC it can release it now
}

float MetalBackend::scaleFactor() const
{
  return [m_view.window backingScaleFactor];
}

float MetalBackend::deltaTime()
{
  const CFAbsoluteTime now { CFAbsoluteTimeGetCurrent() };
  if(!m_lastFrame)
    m_lastFrame = now;
  const float delta { static_cast<float>(now - m_lastFrame) };
  m_lastFrame = now;
  return delta;
}

void MetalBackend::translateAccel(MSG *)
{
  [[m_view window] sendEvent:[NSApp currentEvent]];
}
