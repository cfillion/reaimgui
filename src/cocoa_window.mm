#define CreateEvent SWELL_CreateEvent
#include "window.hpp"

#include "context.hpp"
#include "inputview.hpp"
#include "opengl_renderer.hpp"

#include <WDL/wdltypes.h>
#include <reaper_plugin_functions.h>
#undef CreateEvent

#include <Carbon/Carbon.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

struct Window::Impl {
  static int translateAccel(MSG *msg, accelerator_register_t *accel);

  Context *ctx;
  HwndPtr hwnd;
  NSView *view;
  InputView *inputView;
  accelerator_register_t accel { &translateAccel, true, this };
  ImDrawData *lastDrawData     {};
  NSOpenGLContext *gl;
  OpenGLRenderer *renderer;
};

Window::Window(const char *title, RECT rect, Context *ctx)
  : m_impl { std::make_unique<Impl>() }
{
  const NSPoint position {
    static_cast<float>(rect.left), static_cast<float>(rect.top)
  };
  const NSSize size {
    static_cast<float>(rect.right - rect.left),
    static_cast<float>(rect.bottom - rect.top)
  };

  HWND hwnd { createSwellDialog(title) };
  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
  ShowWindow(hwnd, SW_SHOW);

  m_impl->ctx = ctx;
  m_impl->hwnd = HwndPtr { hwnd };
  m_impl->view = (__bridge NSView *)hwnd; // SWELL_hwndChild inherits from NSView
  [[m_impl->view window] setColorSpace:[NSColorSpace sRGBColorSpace]];
  [[m_impl->view window] setFrameOrigin:position];
  [[m_impl->view window] setContentSize:size];

  constexpr NSOpenGLPixelFormatAttribute attrs[] {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFADoubleBuffer,
    kCGLPFASupportsAutomaticGraphicsSwitching,
    0
  };

  NSOpenGLPixelFormat *fmt { [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] };
  m_impl->gl = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
  if(!m_impl->gl)
    throw reascript_error { "failed to initialize OpenGL 3.2 core context" };

  [m_impl->view setWantsBestResolutionOpenGLSurface:YES]; // retina
  [m_impl->gl setView:m_impl->view];

  [m_impl->gl makeCurrentContext];
  m_impl->renderer = new OpenGLRenderer;
  [m_impl->gl flushBuffer]; // avoid a quick flash of undefined pixels
  [NSOpenGLContext clearCurrentContext];

  m_impl->inputView = [[InputView alloc] initWithContext:ctx parent:m_impl->view];

  // Temprarily enable repeat character input
  // WARNING: this is application-wide!
  NSUserDefaults *defaults { [NSUserDefaults standardUserDefaults] };
  [defaults registerDefaults:@{@"ApplePressAndHoldEnabled":@NO}];

  plugin_register("accelerator", &m_impl->accel);

  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_cocoa";
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

Window::~Window()
{
  plugin_register("-accelerator", &m_impl->accel);

  [m_impl->gl makeCurrentContext];
  delete m_impl->renderer;
}

HWND Window::nativeHandle() const
{
  return m_impl->hwnd.get();
}

void Window::beginFrame()
{
  m_impl->lastDrawData = nullptr;
}

void Window::drawFrame(ImDrawData *drawData)
{
  m_impl->lastDrawData = drawData;
  [m_impl->gl makeCurrentContext];
  m_impl->renderer->draw(drawData, m_impl->ctx->clearColor());
  [m_impl->gl flushBuffer];
  [NSOpenGLContext clearCurrentContext];
}

void Window::endFrame()
{
}

float Window::scaleFactor() const
{
  return [[m_impl->view window] backingScaleFactor];
}

bool Window::handleMessage(const unsigned int msg, WPARAM, LPARAM)
{
  switch(msg) {
  case WM_SIZE:
    [m_impl->gl update];
    if(m_impl->lastDrawData)
      drawFrame(m_impl->lastDrawData);
    return true;
  }

  return false;
}

int Window::Impl::translateAccel(MSG *msg, accelerator_register_t *accel)
{
  enum { NotOurWindow = 0, EatKeystroke = 1 };

  auto *impl { static_cast<Window::Impl *>(accel->user) };
  if(impl->hwnd.get() != msg->hwnd && !IsChild(impl->hwnd.get(), msg->hwnd))
    return NotOurWindow;

  [[impl->view window] sendEvent:[NSApp currentEvent]];

  return EatKeystroke;
}
