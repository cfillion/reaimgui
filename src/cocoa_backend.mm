#include "backend.hpp"

#define CreateEvent SWELL_CreateEvent
#include "context.hpp"
#include "inputview.hpp"
#include "opengl_renderer.hpp"
#undef CreateEvent

#include <Carbon/Carbon.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

class CocoaBackend : public Backend {
public:
  CocoaBackend(Context *);
  ~CocoaBackend() override;

  void beginFrame() override;
  void drawFrame(ImDrawData *) override;
  void resize() override;
  float deltaTime() override;
  float scaleFactor() const override;
  void translateAccel(MSG *) override;

private:
  Context *m_ctx;
  NSView *m_view;
  InputView *m_inputView;
  CFAbsoluteTime m_lastFrame;
  ImDrawData *m_lastDrawData;
  NSOpenGLContext *m_gl;
  std::unique_ptr<OpenGLRenderer> m_renderer;
};

std::unique_ptr<Backend> Backend::create(Context *ctx)
{
  return std::make_unique<CocoaBackend>(ctx);
}

CocoaBackend::CocoaBackend(Context *ctx)
  : m_ctx { ctx }, m_view { (__bridge NSView *)ctx->handle() },
    m_lastFrame {}, m_lastDrawData {}
{
  // Temprarily enable repeat character input
  // WARNING: this is application-wide!
  NSUserDefaults *defaults { [NSUserDefaults standardUserDefaults] };
  [defaults registerDefaults:@{@"ApplePressAndHoldEnabled":@NO}];

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

  const NSOpenGLPixelFormatAttribute attrs[] {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFADoubleBuffer,
    kCGLPFASupportsAutomaticGraphicsSwitching,
    0
  };

  NSOpenGLPixelFormat *fmt { [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] };
  m_gl = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
  [m_view setWantsBestResolutionOpenGLSurface:YES];
  [m_gl setView:m_view];
  [m_gl makeCurrentContext];
  m_renderer = std::make_unique<OpenGLRenderer>();
}

CocoaBackend::~CocoaBackend()
{
}

void CocoaBackend::beginFrame()
{
  m_lastDrawData = nullptr;
}

void CocoaBackend::drawFrame(ImDrawData *drawData)
{
  m_lastDrawData = drawData;
  [m_gl makeCurrentContext];
  m_renderer->draw(drawData, m_ctx->clearColor());
  [m_gl flushBuffer];
}

void CocoaBackend::resize()
{
  [m_gl update];

  if(m_lastDrawData)
    drawFrame(m_lastDrawData);
}

float CocoaBackend::scaleFactor() const
{
  return [m_view.window backingScaleFactor];
}

float CocoaBackend::deltaTime()
{
  const CFAbsoluteTime now { CFAbsoluteTimeGetCurrent() };
  if(!m_lastFrame)
    m_lastFrame = now;
  const float delta { static_cast<float>(now - m_lastFrame) };
  m_lastFrame = now;
  return delta;
}

void CocoaBackend::translateAccel(MSG *)
{
  [[m_view window] sendEvent:[NSApp currentEvent]];
}
