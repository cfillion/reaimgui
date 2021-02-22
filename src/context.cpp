#include "context.hpp"

#include "window.hpp"

// #include <reaper_colortheme.h>
#include <cassert>
#include <reaper_plugin_functions.h>
#include <reaper_plugin_secrets.h>
#include <unordered_set>

struct Heartbeat {
  Heartbeat()
  {
    plugin_register("timer", reinterpret_cast<void *>(&Context::heartbeat));
  }

  ~Heartbeat()
  {
    plugin_register("-timer", reinterpret_cast<void *>(&Context::heartbeat));
  }
};

static std::unordered_set<Context *> g_ctx;

bool Context::exists(Context *win)
{
  return g_ctx.count(win) > 0;
}

size_t Context::count()
{
  return g_ctx.size();
}

void Context::heartbeat()
{
  auto it = g_ctx.begin();

  while(it != g_ctx.end()) {
    Context *ctx = *it++;

    if(ctx->m_closeReq)
      ctx->m_closeReq = false;

    if(ctx->m_inFrame)
      ctx->endFrame(true);
    else
      delete ctx;
  }
}

Context::Context(const char *title,
    const int x, const int y, const int w, const int h)
  : m_inFrame { false }, m_closeReq { false }, m_cursor {}, m_mouseDown {},
    m_lastFrame { decltype(m_lastFrame)::clock::now() },
    m_imgui { nullptr, &ImGui::DestroyContext }
{
  static std::weak_ptr<Heartbeat> g_heartbeat;

  if(g_heartbeat.expired())
    g_heartbeat = m_heartbeat = std::make_shared<Heartbeat>();
  else
    m_heartbeat = g_heartbeat.lock();

  setupImGui();

  const RECT rect { x, y, x + w, y + h };
  m_window = std::make_unique<Window>(title, rect, this);
  AttachWindowTopmostButton(m_window->nativeHandle());
  g_ctx.emplace(this);
}

void Context::setupImGui()
{
  static std::weak_ptr<ImFontAtlas> g_fontAtlas;

  if(g_fontAtlas.expired())
    g_fontAtlas = m_fontAtlas = std::make_shared<ImFontAtlas>();
  else
    m_fontAtlas = g_fontAtlas.lock();

  m_imgui.reset(ImGui::CreateContext(m_fontAtlas.get()));
  ImGui::SetCurrentContext(m_imgui.get());

  ImGuiIO &io { ImGui::GetIO() };
  io.IniFilename = nullptr;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

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
}

Context::~Context()
{
  ImGui::SetCurrentContext(m_imgui.get());

  if(m_inFrame)
    endFrame(false);

  g_ctx.erase(this);
}

void Context::beginFrame()
{
  assert(!m_inFrame);

  m_inFrame = true;

  updateFrameInfo();
  updateTheme();
  updateMouseDown();
  updateMousePos();
  updateKeyMods();

  ImGui::NewFrame();
  m_window->beginFrame();
}

void Context::enterFrame()
{
  ImGui::SetCurrentContext(m_imgui.get());

  if(!m_inFrame)
    beginFrame();
}

void Context::endFrame(const bool render) try
{
  ImGui::SetCurrentContext(m_imgui.get());

  if(render) {
    updateCursor();
    ImGui::Render();
    m_window->drawFrame(ImGui::GetDrawData());
  }
  else
    ImGui::EndFrame();

  m_window->endFrame();
  m_inFrame = false;
}
catch(const imgui_error &e) {
  char message[1024];
  snprintf(message, sizeof(message), "ImGui assertion failed: %s\n", e.what());
  ShowConsoleMsg(message); // cannot use ReaScriptError unless called by a script

  // no recovery, just destroy the context
  m_window->endFrame();
  m_fontAtlas->Locked = false; // don't assert again when destroying the atlas
  m_inFrame = false; // don't call endFrame again from the destructor
  delete this;
}

void Context::updateFrameInfo()
{
  ImGuiIO &io { ImGui::GetIO() };

  RECT rect;
  GetClientRect(m_window->nativeHandle(), &rect);
  io.DisplaySize = ImVec2(rect.right - rect.left, rect.bottom - rect.top);

  const float scale { m_window->scaleFactor() };
  io.DisplayFramebufferScale = ImVec2{scale, scale};

  const auto now { decltype(m_lastFrame)::clock::now() };
  io.DeltaTime = std::chrono::duration<float> { now - m_lastFrame }.count();
  m_lastFrame = now;
}

void Context::updateCursor()
{
  ImGui::SetCurrentContext(m_imgui.get());

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

  // ImGui::GetMouseCursor is only valid after a frame, before Render
  // (it's reset when a new frame is started)
  const ImGuiMouseCursor imguiCursor { ImGui::GetMouseCursor() };
  const bool hidden { imguiCursor == ImGuiMouseCursor_None };
  HCURSOR cursor { hidden ? nullptr : nativeCursors[imguiCursor] };
  if(m_cursor != cursor)
    SetCursor(m_cursor = cursor);
}

bool Context::anyMouseDown() const
{
  for(auto state : m_mouseDown) {
    if(state & Down)
      return true;
  }

  return false;
}

void Context::mouseDown(const unsigned int msg)
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
    SetCapture(m_window->nativeHandle());
#endif

  m_mouseDown[btn] = Down | DownUnread;
}

void Context::mouseUp(const unsigned int msg)
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
  if(!anyMouseDown() && GetCapture() == m_window->nativeHandle())
    ReleaseCapture();
#endif
}

void Context::updateMouseDown()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_imgui.get());

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
  // ImGui::SetCurrentContext(m_imgui.get());

  HWND windowHwnd { m_window->nativeHandle() };

  POINT p;
  GetCursorPos(&p);
  const HWND targetHwnd { WindowFromPoint(p) };
  ScreenToClient(windowHwnd, &p);

  ImGuiIO &io { ImGui::GetIO() };

#ifdef __APPLE__
  // Our InputView overlays SWELL's NSView.
  // Capturing is not used as macOS sends mouse up events from outside of the
  // frame when the mouse down event occured within.
  if(IsChild(windowHwnd, targetHwnd) || anyMouseDown())
#else
  if(targetHwnd == windowHwnd || GetCapture() == windowHwnd)
#endif
    io.MousePos = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
  else
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
}

void Context::mouseWheel(const unsigned int msg, const short delta)
{
  ImGui::SetCurrentContext(m_imgui.get());

#ifndef WHEEL_DELTA
  constexpr float WHEEL_DELTA {
#  ifdef __APPLE__
    60.0f
#  else
    120.0f
#  endif
  };
#endif

  ImGuiIO &io { ImGui::GetIO() };
  float &wheel { msg == WM_MOUSEHWHEEL ? io.MouseWheelH : io.MouseWheel };
  wheel += static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
}

void Context::updateKeyMods()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_imgui.get());

  constexpr int down { 0x8000 };

  ImGuiIO &io { ImGui::GetIO() };
  io.KeyCtrl  = GetAsyncKeyState(VK_CONTROL) & down;
  io.KeyShift = GetAsyncKeyState(VK_SHIFT)   & down;
  io.KeyAlt   = GetAsyncKeyState(VK_MENU)    & down;
  io.KeySuper = GetAsyncKeyState(VK_LWIN)    & down;
}

void Context::keyInput(const uint8_t key, const bool down)
{
  ImGui::SetCurrentContext(m_imgui.get());
  ImGuiIO &io { ImGui::GetIO() };
  io.KeysDown[key] = down;
}

void Context::charInput(const unsigned int codepoint)
{
  if(codepoint < 32 || (codepoint > 126 && codepoint < 160))
    return;

  ImGui::SetCurrentContext(m_imgui.get());
  ImGuiIO &io { ImGui::GetIO() };
  io.AddInputCharacter(codepoint);
}

void Context::updateTheme()
{
  m_clearColor = Color::fromTheme(GSC_mainwnd(COLOR_WINDOW));
}
