#include "context.hpp"

#include "backend.hpp"
#include "watchdog.hpp"

#include <imgui/imgui_internal.h>
#include <reaper_colortheme.h>
#include <reaper_plugin_functions.h>
#include <stdexcept>
#include <unordered_set>

static std::unordered_set<Context *> g_windows;
static std::weak_ptr<ImFontAtlas> g_fontAtlas;

REAPER_PLUGIN_HINSTANCE Context::s_instance;

#ifndef GET_WHEEL_DELTA_WPARAM
#  define GET_WHEEL_DELTA_WPARAM GET_Y_LPARAM
#endif

#ifndef WHEEL_DELTA
constexpr float WHEEL_DELTA {
#  ifdef __APPLE__
  60.0f
#  else
  120.0f
#  endif
};
#endif

enum SwellDialogResFlags {
  ForceNonChild = 0x400000 | 0x8, // allows not using a resource id
  Resizable = 1,
};

static void reportRecovery(void *, const char *fmt, ...)
{
  char msg[1024];

  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  ReaScriptError(msg);
  fprintf(stderr, "ReaImGUI Warning: %s\n", msg);
}

WDL_DLGRET Context::proc(HWND handle, const UINT msg,
  const WPARAM wParam, const LPARAM lParam)
{
  Context *self {
    reinterpret_cast<Context *>(GetWindowLongPtr(handle, GWLP_USERDATA))
  };

  if(!self)
    return 0;

  switch(msg) {
  case WM_CLOSE:
    self->m_closeReq = true;
    break;
  case WM_DESTROY:
    delete self;
    break;
  case WM_MOUSEMOVE:
    self->updateCursor();
    break;
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
    self->mouseWheel(msg, GET_WHEEL_DELTA_WPARAM(wParam));
    break;
  case WM_SETCURSOR:
    return 1;
#ifndef __APPLE__ // these are handled by InputView, bypassing SWELL
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    self->mouseDown(msg);
    break;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    self->mouseUp(msg);
    break;
#endif // __APPLE__
#ifdef _WIN32
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    if(wParam < 256)
      self->keyInput(wParam, true);
    return -1;
  case WM_KEYUP:
  case WM_SYSKEYUP:
    if(wParam < 256)
      self->keyInput(wParam, false);
    return -1;
#endif // _WIN32
  case WM_SIZE:
    self->m_backend->resize();
    break;
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

int Context::translateAccel(MSG *msg, accelerator_register_t *accel)
{
  enum { NotOurWindow = 0, EatKeystroke = 1 };

  Context *self { static_cast<Context *>(accel->user) };
  if(self->handle() != msg->hwnd && !IsChild(self->handle(), msg->hwnd))
    return NotOurWindow;

  self->m_backend->translateAccel(msg);

  return EatKeystroke;
}

bool Context::exists(Context *win)
{
  return g_windows.count(win) > 0;
}

size_t Context::count()
{
  return g_windows.size();
}

void Context::heartbeat()
{
  auto it = g_windows.begin();

  while(it != g_windows.end()) {
    Context *win = *it++;

    if(win->m_keepAlive)
      win->m_keepAlive = false;
    else
      win->close();
  }
}

Context::Context(const char *title,
    const int x, const int y, const int w, const int h)
  : m_keepAlive { true }, m_inFrame { false }, m_closeReq { false },
    m_clearColor { 0x000000FF }, m_mouseDown {},
    m_accel { &Context::translateAccel, true, this },
    m_watchdog { Watchdog::get() }
{
  m_handle = CreateDialog(s_instance,
    MAKEINTRESOURCE(ForceNonChild | Resizable),
    GetMainHwnd(), proc);

  SetWindowText(m_handle, title);
  SetWindowPos(m_handle, HWND_TOP, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
  ShowWindow(m_handle, SW_SHOW);

  setupImGui();

  try {
    m_backend = Backend::create(this);
  }
  catch(const std::runtime_error &) {
    ImGui::DestroyContext();
    DestroyWindow(m_handle);
    throw;
  }

  SetWindowLongPtr(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  plugin_register("accelerator", &m_accel);
  g_windows.emplace(this);
}

void Context::setupImGui()
{
  if(g_fontAtlas.expired())
    g_fontAtlas = m_fontAtlas = std::make_shared<ImFontAtlas>();
  else
    m_fontAtlas = g_fontAtlas.lock();

  m_imgui = ImGui::CreateContext(m_fontAtlas.get());
  ImGui::SetCurrentContext(m_imgui);
  ImGui::StyleColorsDark();

  ImGuiIO &io { ImGui::GetIO() };
  io.IniFilename = nullptr;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendPlatformName = "reaper_imgui";

#ifndef __APPLE__
  io.KeyMap[ImGuiKey_Tab]         = VK_TAB;
  io.KeyMap[ImGuiKey_LeftArrow]   = VK_LEFT;
  io.KeyMap[ImGuiKey_RightArrow]  = VK_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow]     = VK_UP;
  io.KeyMap[ImGuiKey_DownArrow]   = VK_DOWN;
  io.KeyMap[ImGuiKey_PageUp]      = VK_PRIOR;
  io.KeyMap[ImGuiKey_PageDown]    = VK_NEXT;
  io.KeyMap[ImGuiKey_Home]        = VK_HOME;
  io.KeyMap[ImGuiKey_End]         = VK_END;
  io.KeyMap[ImGuiKey_Insert]      = VK_INSERT;
  io.KeyMap[ImGuiKey_Delete]      = VK_DELETE;
  io.KeyMap[ImGuiKey_Backspace]   = VK_BACK;
  io.KeyMap[ImGuiKey_Space]       = VK_SPACE;
  io.KeyMap[ImGuiKey_Enter]       = VK_RETURN;
  io.KeyMap[ImGuiKey_Escape]      = VK_ESCAPE;
  io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
  io.KeyMap[ImGuiKey_A]           = 'A';
  io.KeyMap[ImGuiKey_C]           = 'C';
  io.KeyMap[ImGuiKey_V]           = 'V';
  io.KeyMap[ImGuiKey_X]           = 'X';
  io.KeyMap[ImGuiKey_Y]           = 'Y';
  io.KeyMap[ImGuiKey_Z]           = 'Z';
#endif

  int themeSize;
  ColorTheme *theme { static_cast<ColorTheme *>(GetColorThemeStruct(&themeSize)) };
  if(static_cast<size_t>(themeSize) >= sizeof(ColorTheme))
    m_clearColor = Color(theme->main_bg, false);
}

Context::~Context()
{
  ImGui::SetCurrentContext(m_imgui);

  plugin_register("-accelerator", &m_accel);

  if(m_inFrame)
    endFrame(false);

  m_backend.reset(); // destroy the backend before ImGui
  ImGui::DestroyContext();

  g_windows.erase(this);
}

void Context::beginFrame()
{
  assert(!m_inFrame);

  m_inFrame = true;
  updateFrameInfo(); // before calling the backend
  m_backend->beginFrame();
  updateMouseDown();
  updateMousePos();
  updateKeyMods();
  ImGui::NewFrame();
}

void Context::enterFrame()
{
  ImGui::SetCurrentContext(m_imgui);

  if(!m_inFrame)
    beginFrame();
}

void Context::endFrame(const bool render)
{
  ImGui::SetCurrentContext(m_imgui);
  ImGui::ErrorCheckEndFrameRecover(reportRecovery);

  if(render) {
    ImGui::Render();
    m_backend->drawFrame(ImGui::GetDrawData());
  }
  else
    ImGui::EndFrame();

  m_inFrame = false;
  m_keepAlive = true;

  m_backend->endFrame();
}

void Context::close()
{
  DestroyWindow(m_handle);
}

void Context::updateFrameInfo()
{
  ImGuiIO &io { ImGui::GetIO() };

  RECT rect;
  GetClientRect(m_handle, &rect);
  io.DisplaySize = ImVec2(rect.right - rect.left, rect.bottom - rect.top);

  const float scale { m_backend->scaleFactor() };
  io.DisplayFramebufferScale = ImVec2{scale, scale};

  io.DeltaTime = m_backend->deltaTime();
}

void Context::updateCursor()
{
  ImGui::SetCurrentContext(m_imgui);

  static HCURSOR nativeCursors[ImGuiMouseCursor_COUNT] {
    LoadCursor(nullptr, IDC_ARROW),
    LoadCursor(nullptr, IDC_IBEAM),
    LoadCursor(nullptr, IDC_SIZEALL),
    LoadCursor(nullptr, IDC_SIZENS),
    LoadCursor(nullptr, IDC_SIZEWE),
    LoadCursor(nullptr, IDC_SIZENESW),
    LoadCursor(nullptr, IDC_SIZENWSE),
    LoadCursor(nullptr, IDC_HAND),
    LoadCursor(nullptr, IDC_NO),
  };

  // TODO
  // io.MouseDrawCursor (ImGui-drawn cursor)
  // ImGuiConfigFlags_NoMouseCursorChange
  // SetCursor(nullptr) does not hide the cursor with SWELL

  const ImGuiMouseCursor imguiCursor { ImGui::GetMouseCursor() };
  const bool hidden { imguiCursor == ImGuiMouseCursor_None };
  SetCursor(hidden ? nullptr : nativeCursors[imguiCursor]);
}

bool Context::anyMouseDown() const
{
  for(auto state : m_mouseDown) {
    if(state & Down)
      return true;
  }

  return false;
}

void Context::mouseDown(const UINT msg)
{
  size_t btn;

  switch(msg) {
  case WM_LBUTTONDOWN:
    btn = ImGuiMouseButton_Left;
    break;
  case WM_MBUTTONDOWN:
    btn = ImGuiMouseButton_Middle;
    break;
  case WM_RBUTTONDOWN:
    btn = ImGuiMouseButton_Right;
    break;
  default:
    return;
  }

#ifndef __APPLE__
  if(!anyMouseDown() && GetCapture() == nullptr)
    SetCapture(m_handle);
#endif

  m_mouseDown[btn] = Down | DownUnread;
}

void Context::mouseUp(const UINT msg)
{
  size_t btn;

  switch(msg) {
  case WM_LBUTTONUP:
    btn = ImGuiMouseButton_Left;
    break;
  case WM_MBUTTONUP:
    btn = ImGuiMouseButton_Middle;
    break;
  case WM_RBUTTONUP:
    btn = ImGuiMouseButton_Right;
    break;
  default:
    return;
  }

  // keep DownUnread set to catch clicks shorted than one frame
  m_mouseDown[btn] &= ~Down;

#ifndef __APPLE__
  if(!anyMouseDown() && GetCapture() == m_handle)
    ReleaseCapture();
#endif
}

void Context::updateMouseDown()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_imgui);

  ImGuiIO &io { ImGui::GetIO() };

  size_t i {};
  for(auto &state : m_mouseDown) {
    io.MouseDown[i++] = (state & DownUnread) || (state & Down);
    state &= ~DownUnread;
  }
}

void Context::updateMousePos()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_imgui);

  POINT p;
  GetCursorPos(&p);
  const HWND targetView { WindowFromPoint(p) };
  ScreenToClient(m_handle, &p);

  ImGuiIO &io { ImGui::GetIO() };

#ifdef __APPLE__
  // Our InputView overlays SWELL's NSView.
  // Capturing is not used as macOS sends mouse up events from outside of the
  // frame when the mouse down event occured within.
  if(IsChild(m_handle, targetView) || anyMouseDown())
#else
  if(targetView == m_handle || GetCapture() == m_handle)
#endif
    io.MousePos = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
  else
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
}

void Context::mouseWheel(const UINT msg, const short delta)
{
  ImGui::SetCurrentContext(m_imgui);
  ImGuiIO &io { ImGui::GetIO() };
  float &wheel { msg == WM_MOUSEHWHEEL ? io.MouseWheelH : io.MouseWheel };
  wheel += static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
}

void Context::updateKeyMods()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_imgui);

  constexpr int down { 0x8000 };

  ImGuiIO &io { ImGui::GetIO() };
  io.KeyCtrl  = GetAsyncKeyState(VK_CONTROL) & down;
  io.KeyShift = GetAsyncKeyState(VK_SHIFT)   & down;
  io.KeyAlt   = GetAsyncKeyState(VK_MENU)    & down;
  io.KeySuper = GetAsyncKeyState(VK_LWIN)    & down;
}

void Context::keyInput(const uint8_t key, const bool down)
{
  ImGui::SetCurrentContext(m_imgui);
  ImGuiIO &io { ImGui::GetIO() };
  io.KeysDown[key] = down;
}

void Context::charInput(const unsigned int codepoint)
{
  if(codepoint < 32 || (codepoint > 126 && codepoint < 160))
    return;

  ImGui::SetCurrentContext(m_imgui);
  ImGuiIO &io { ImGui::GetIO() };
  io.AddInputCharacter(codepoint);
}
