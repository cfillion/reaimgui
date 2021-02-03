#include "window.hpp"

#include "watchdog.hpp"

#include <imgui/imgui.h>
#include <reaper_colortheme.h>
#include <reaper_plugin_functions.h>
#include <unordered_set>

static std::unordered_set<Window *> g_windows;
static std::weak_ptr<ImFontAtlas> g_fontAtlas;

REAPER_PLUGIN_HINSTANCE Window::s_instance;

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
    m_p { nullptr }, m_watchdog { Watchdog::get() }
{
  g_windows.emplace(this);

  m_handle = CreateDialogParam(s_instance,
    MAKEINTRESOURCE(ForceNonChild | Resizable),
    GetMainHwnd(),
    proc, reinterpret_cast<LPARAM>(this));

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
