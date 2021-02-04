#include "window.hpp"

#include "watchdog.hpp"

#include <reaper_colortheme.h>
#include <reaper_plugin_functions.h>
#include <unordered_set>

static std::unordered_set<Window *> g_windows;
static std::weak_ptr<ImFontAtlas> g_fontAtlas;

REAPER_PLUGIN_HINSTANCE Window::s_instance;

#ifdef _WDL_SWELL_H_
#  define GET_WHEEL_DELTA_WPARAM GET_Y_LPARAM
#  define WHEEL_DELTA 60.0f
#endif

enum SwellDialogResFlags {
  ForceNonChild = 0x400000 | 0x8, // allows not using a resource id
  Resizable = 1,
};

WDL_DLGRET Window::proc(HWND handle, const UINT msg,
  const WPARAM wParam, const LPARAM lParam)
{
  Window *self {
    reinterpret_cast<Window *>(GetWindowLongPtr(handle, GWLP_USERDATA))
  };

  if(!self)
    return false;

  switch(msg) {
  case WM_CLOSE:
    self->m_closeReq = true;
    break;
  case WM_DESTROY:
    delete self;
    break;
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
  case WM_MOUSEMOVE:
    self->updateCursor();
    break;
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
    self->mouseWheel(msg, GET_WHEEL_DELTA_WPARAM(wParam));
    break;
  case WM_SETCURSOR:
    return true;
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

bool Window::exists(Window *win)
{
  return g_windows.count(win) > 0;
}

size_t Window::count()
{
  return g_windows.size();
}

void Window::heartbeat()
{
  auto it = g_windows.begin();

  while(it != g_windows.end()) {
    Window *win = *it++;

    if(win->m_keepAlive)
      win->m_keepAlive = false;
    else
      win->close();
  }
}

Window::Window(const char *title,
    const int x, const int y, const int w, const int h)
  : m_keepAlive { true }, m_inFrame { false }, m_closeReq { false },
    m_clearColor { std::make_tuple(0.0f, 0.0f, 0.0f, 1.0f) },
    m_mouseDown {}, m_p { nullptr }, m_watchdog { Watchdog::get() }
{
  g_windows.emplace(this);

  m_handle = CreateDialog(s_instance,
    MAKEINTRESOURCE(ForceNonChild | Resizable),
    GetMainHwnd(), proc);

  SetWindowLongPtr(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  SetWindowText(m_handle, title);
  SetWindowPos(m_handle, HWND_TOP, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
  ShowWindow(m_handle, SW_SHOW);

  setupContext();
  platformInit();
}

void Window::setupContext()
{
  if(g_fontAtlas.expired())
    g_fontAtlas = m_fontAtlas = std::make_shared<ImFontAtlas>();
  else
    m_fontAtlas = g_fontAtlas.lock();

  m_ctx = ImGui::CreateContext(m_fontAtlas.get());
  ImGui::SetCurrentContext(m_ctx);
  ImGui::StyleColorsDark();

  ImGuiIO &io { ImGui::GetIO() };
  io.IniFilename = nullptr;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendPlatformName = "reaper_imgui";

  int themeSize;
  ColorTheme *theme { static_cast<ColorTheme *>(GetColorThemeStruct(&themeSize)) };
  if(static_cast<size_t>(themeSize) >= sizeof(ColorTheme))
    setClearColor((theme->main_bg << 8) | 0xff);
}

Window::~Window()
{
  ImGui::SetCurrentContext(m_ctx);

  if(m_inFrame)
    endFrame(false);

  platformTeardown();
  ImGui::DestroyContext();

  g_windows.erase(this);
}

void Window::enterFrame()
{
  ImGui::SetCurrentContext(m_ctx);

  if(m_inFrame)
    return;

  m_inFrame = true;
  platformBeginFrame();
  updateMouseDown();
  updateMousePos();
  ImGui::NewFrame();
}

void Window::endFrame(const bool render)
{
  ImGui::SetCurrentContext(m_ctx);

  ImDrawData *drawData {};

  if(render) {
    ImGui::Render();
    drawData = ImGui::GetDrawData();
  }
  else
    ImGui::EndFrame();

  m_inFrame = false;
  m_keepAlive = true;

  platformEndFrame(drawData);
}

void Window::close()
{
  DestroyWindow(m_handle);
}

unsigned int Window::clearColor() const
{
  const auto [r, g, b, a] { m_clearColor };

  return
    (static_cast<unsigned int>(a * 0xFF)      ) |
    (static_cast<unsigned int>(b * 0xFF) << 8 ) |
    (static_cast<unsigned int>(g * 0xFF) << 16) |
    (static_cast<unsigned int>(r * 0xFF) << 24);
}

void Window::setClearColor(const unsigned int rgba)
{
  const uint8_t
    r = rgba >> 24,
    g = rgba >> 16,
    b = rgba >> 8,
    a = rgba;

  m_clearColor = std::make_tuple(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
}

void Window::updateCursor()
{
  ImGui::SetCurrentContext(m_ctx);

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

bool Window::anyMouseDown() const
{
  for(auto state : m_mouseDown) {
    if(state & Down)
      return true;
  }

  return false;
}

void Window::mouseDown(const UINT msg)
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

  if(!anyMouseDown() && GetCapture() == nullptr)
    SetCapture(m_handle);

  m_mouseDown[btn] = Down | DownUnread;
}

void Window::mouseUp(const UINT msg)
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
  m_mouseDown[ImGuiMouseButton_Left] &= ~Down;

  if(!anyMouseDown() && GetCapture() == m_handle)
    ReleaseCapture();
}

void Window::updateMouseDown()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_ctx);

  ImGuiIO &io { ImGui::GetIO() };

  size_t i {};
  for(auto &state : m_mouseDown) {
    io.MouseDown[i++] = (state & DownUnread) || (state & Down);
    state &= ~DownUnread;
  }
}

void Window::updateMousePos()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_ctx);

  POINT p;
  GetCursorPos(&p);
  const HWND targetWindow { WindowFromPoint(p) };
  ScreenToClient(m_handle, &p);

  ImGuiIO &io { ImGui::GetIO() };

  if(targetWindow == m_handle)
    io.MousePos = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
  else
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
}

void Window::mouseWheel(const UINT msg, const short delta)
{
  ImGui::SetCurrentContext(m_ctx);
  ImGuiIO &io { ImGui::GetIO() };
  float &wheel { msg == WM_MOUSEHWHEEL ? io.MouseWheelH : io.MouseWheel };
  wheel += static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
}
