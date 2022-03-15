/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#include "gdk_window.hpp"

#include "context.hpp"
#include "gdk_event_handler.hpp"
#include "opengl_renderer.hpp"
#include "platform.hpp"

#include <cassert>
#include <epoxy/gl.h>
#include <gtk/gtk.h>

#include <swell/swell.h>
#include <reaper_plugin_functions.h>

#define _LICE_H // prevent swell-internal.h from including lice.h
#define SWELL_LICE_GDI
#define SWELL_TARGET_GDK
#define Font Xorg_Font
#define Window Xorg_Window
#include <swell/swell-internal.h> // access to hwnd->m_oswindow
#undef Window

GDKWindow::GDKWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
  : Window { viewport, dockerHost }, m_gl { nullptr }, m_ime { nullptr }
{
  static std::weak_ptr<GdkEventHandler> g_eventHandler;

  if(g_eventHandler.expired())
    g_eventHandler = m_eventHandler = std::make_shared<GdkEventHandler>();
  else
    m_eventHandler = g_eventHandler.lock();
}

void GDKWindow::create()
{
  createSwellDialog();
  SetWindowLongPtr(m_hwnd.get(), GWL_EXSTYLE, WS_EX_ACCEPTFILES);

  // WS_CHILD does gdk_window_set_override_redirect(true)
  // SWELL only supports setting WS_CHILD before ShowWindow
  if(m_viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
    SetWindowLongPtr(m_hwnd.get(), GWL_STYLE, WS_CHILD);

  m_previousFlags = ~m_viewport->Flags; // update will be called before show
}

GDKWindow::~GDKWindow()
{
  if(m_gl)
    teardownGl();
  if(m_ime)
    g_object_unref(m_ime);
}

void GDKWindow::LICEDeleter::operator()(LICE_IBitmap *bm)
{
  LICE__Destroy(bm);
}

void GDKWindow::initSoftwareBlit()
{
  m_pixels.reset(LICE_CreateBitmap(0, 0, 0));

  static std::weak_ptr<GdkWindow> g_offscreen;

  if(g_offscreen.expired()) {
    GdkWindowAttr attr {};
    attr.window_type = GDK_WINDOW_TOPLEVEL;
    GdkWindow *window { gdk_window_new(nullptr, &attr, 0) };
    std::shared_ptr<GdkWindow> offscreen { window, g_object_unref };
    g_offscreen = m_offscreen = offscreen;
  }
  else
    m_offscreen = g_offscreen.lock();
}

void GDKWindow::initGl()
{
  GdkWindow *window { m_offscreen ? m_offscreen.get() : m_hwnd->m_oswindow };

  if(static_cast<void *>(window) == m_hwnd.get())
    throw backend_error { "headless SWELL is not supported" };

  GError *error {};
  m_gl = gdk_window_create_gl_context(window, &error);
  if(error) {
    const backend_error ex { error->message };
    g_clear_error(&error);
    assert(!m_gl);
    throw ex;
  }

  gdk_gl_context_set_required_version(m_gl,
    OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR);
  gdk_gl_context_set_forward_compatible(m_gl, true);

  gdk_gl_context_realize(m_gl, &error);
  if(error) {
    const backend_error ex { error->message };
    g_clear_error(&error);
    g_clear_object(&m_gl);
    throw ex;
  }

  gdk_gl_context_make_current(m_gl);

  int major, minor;
  gdk_gl_context_get_version(m_gl, &major, &minor);
  if(major < OpenGLRenderer::MIN_MAJOR ||
      (major == OpenGLRenderer::MIN_MAJOR && minor < OpenGLRenderer::MIN_MINOR)) {
    gdk_gl_context_clear_current();
    g_clear_object(&m_gl);

    char msg[1024];
    snprintf(msg, sizeof(msg), "OpenGL v%d.%d or newer required, got v%d.%d",
      OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR, major, minor);
    throw backend_error { msg };
  }

  glGenTextures(1, &m_tex);
  resizeTextures(); // binds to the texture and sets its size

  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  m_renderer = new OpenGLRenderer;

  gdk_gl_context_clear_current();
}

void GDKWindow::resizeTextures()
{
  RECT rect;
  GetClientRect(m_hwnd.get(), &rect);
  const int width  { rect.right - rect.left },
            height { rect.bottom - rect.top };

  glBindTexture(GL_TEXTURE_2D, m_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
    0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

  if(m_pixels) {
    LICE__resize(m_pixels.get(), width, height);
    glPixelStorei(GL_PACK_ROW_LENGTH, LICE__GetRowSpan(m_pixels.get()));
  }
}

void GDKWindow::teardownGl()
{
  gdk_gl_context_make_current(m_gl);

  glDeleteFramebuffers(1, &m_fbo);
  glDeleteTextures(1, &m_tex);

  delete m_renderer;

  // current GL context must be cleared before calling unref to avoid this bug:
  // https://gitlab.gnome.org/GNOME/gtk/-/issues/2562
  gdk_gl_context_clear_current();
  g_object_unref(m_gl);
}

static void imeCommit(GtkIMContext *, gchar *input, gpointer data)
{
  Context *ctx { reinterpret_cast<Context *>(data) };
  while(*input) {
    ctx->charInput(g_utf8_get_char(input));
    input = g_utf8_next_char(input);
  }
}

void GDKWindow::initIME()
{
  m_ime = gtk_im_multicontext_new();
  gtk_im_context_set_use_preedit(m_ime, false);
  gtk_im_context_focus_in(m_ime);
  g_signal_connect(m_ime, "commit", G_CALLBACK(imeCommit), m_ctx);
}

void GDKWindow::show()
{
  Window::show();

  if(isDocked())
    initSoftwareBlit();

  initGl();

  // prevent invalidation (= displaying garbage) when moving another window over
  if(!isDocked())
    gdk_window_freeze_updates(m_hwnd->m_oswindow);

  initIME();
}

void GDKWindow::setPosition(ImVec2 pos)
{
  Platform::scalePosition(&pos, true);
  SetWindowPos(m_hwnd.get(), nullptr, pos.x, pos.y, 0, 0,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void GDKWindow::setSize(const ImVec2 size)
{
  SetWindowPos(m_hwnd.get(), nullptr, 0, 0,
    size.x * scaleFactor(), size.y * scaleFactor(),
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void GDKWindow::setTitle(const char *title)
{
  SetWindowText(m_hwnd.get(), title);
}

void GDKWindow::update()
{
  if(GetFocus() == m_hwnd.get())
    SWELL_SetClassName(m_hwnd.get(), getSwellClass());

  if(isDocked())
    return;

  const ImGuiViewportFlags diff { m_previousFlags ^ m_viewport->Flags };
  m_previousFlags = m_viewport->Flags;

  if(diff & ImGuiViewportFlags_NoDecoration) {
    auto style { GetWindowLongPtr(m_hwnd.get(), GWL_STYLE) };

    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration)
      style &= ~WS_CAPTION;
    else
      style |= WS_CAPTION;

    SetWindowLongPtr(m_hwnd.get(), GWL_STYLE, style);

    // SetWindowLongPtr hides the window
    // it sets an internal "need show" flag that's used by SetWindowPos
    if(m_hwnd->m_oswindow) {
      setPosition(m_viewport->Pos);
      setSize(m_viewport->Size);
    }
  }

  if(diff & ImGuiViewportFlags_TopMost) {
    if(m_viewport->Flags & ImGuiViewportFlags_TopMost)
      SWELL_SetWindowLevel(m_hwnd.get(), 1);
    else
      SWELL_SetWindowLevel(m_hwnd.get(), 0);
  }
}

void GDKWindow::render(void *)
{
  gdk_gl_context_make_current(m_gl);

  const bool softwareBlit { isDocked() };
  m_renderer->render(m_viewport, softwareBlit);

  if(softwareBlit) {
    // REAPER is also drawing to the same GdkWindow so we must share it.
    // Switch to slower render path, copying pixels into a LICE bitmap.
    glReadPixels(0, 0,
      LICE__GetWidth(m_pixels.get()), LICE__GetHeight(m_pixels.get()),
      GL_BGRA, GL_UNSIGNED_BYTE, LICE__GetBits(m_pixels.get()));
    InvalidateRect(m_hwnd.get(), nullptr, false);
    gdk_gl_context_clear_current();
    return;
  }

  ImDrawData *drawData { m_viewport->DrawData };

  GdkWindow *window { m_hwnd->m_oswindow };
  const cairo_region_t *region { gdk_window_get_clip_region(window) };
  GdkDrawingContext *drawContext { gdk_window_begin_draw_frame(window, region) };
  cairo_t *cairoContext { gdk_drawing_context_get_cairo_context(drawContext) };
  gdk_cairo_draw_from_gl(cairoContext, window,
    m_tex, GL_TEXTURE, 1, 0, 0,
    drawData->DisplaySize.x * drawData->FramebufferScale.x,
    drawData->DisplaySize.y * drawData->FramebufferScale.y);
  gdk_window_end_draw_frame(window, drawContext);

  // required for making the window visible on GNOME
  gdk_window_thaw_updates(window); // schedules an update
  gdk_window_freeze_updates(window);

  gdk_gl_context_clear_current();
}

void GDKWindow::softwareBlit()
{
  constexpr int LICE_BLIT_MODE_COPY      { 0x00000 },
                LICE_BLIT_IGNORE_SCALING { 0x20000 };

  PAINTSTRUCT ps;
  if(!BeginPaint(m_hwnd.get(), &ps))
    return;

  const int width  { LICE__GetWidth(m_pixels.get())  },
            height { LICE__GetHeight(m_pixels.get()) };

  LICE_Blit(ps.hdc->surface, m_pixels.get(),
    ps.hdc->surface_offs.x, ps.hdc->surface_offs.y,
    0, 0, width, height, 1.0f, LICE_BLIT_MODE_COPY | LICE_BLIT_IGNORE_SCALING);

  EndPaint(m_hwnd.get(), &ps);
}

float GDKWindow::globalScaleFactor()
{
  static float scale { SWELL_GetScaling256() / 256.f };
  return scale;
}

void GDKWindow::setIME(ImGuiPlatformImeData *data)
{
  if(!data->WantVisible)
    gtk_im_context_reset(m_ime);

  // cannot use m_viewport->Pos when docked
  // (IME cursor location must be relative to the dock host window)
  HWND container { m_hwnd.get() };
  while(!container->m_oswindow)
    container = GetParent(container);
  RECT containerPos;
  if(container)
    GetWindowRect(container, &containerPos);
  else {
    containerPos.left = m_viewport->Pos.x;
    containerPos.right = m_viewport->Pos.y;
  }

  GdkRectangle area;
  area.x = data->InputPos.x - containerPos.left;
  area.y = data->InputPos.y - containerPos.top;
  area.width = 0;
  area.height = data->InputLineHeight;
  gtk_im_context_set_cursor_location(m_ime, &area);
}

void GDKWindow::uploadFontTex()
{
  gdk_gl_context_make_current(m_gl);
  m_renderer->uploadFontTex();
  gdk_gl_context_clear_current();
}

static unsigned int unmangleSwellChar(WPARAM wParam, LPARAM lParam)
{
  // Trying to guess the character to print from SWELL's event data.
  // Matching the behavior of OnEditKeyDown from swell-wnd-generic.cpp

  if(lParam & (FCONTROL | FALT | FLWIN) || wParam < 32)
    return 0;

  if(wParam >= 'A' && wParam <= 'Z') {
    // This does not support caps lock.
    if(!(lParam & FSHIFT))
      wParam += 'a' - 'A';
  }
  else if(wParam >= VK_NUMPAD0 && wParam <= VK_DIVIDE) {
    if(wParam <= VK_NUMPAD9)
      wParam += '0' - VK_NUMPAD0;
    else
      wParam += '*' - VK_MULTIPLY;
  }
  else if(lParam & FVIRTKEY && (wParam < '0' || wParam > '9') && wParam != VK_SPACE)
    return 0; // virtual keys that aren't letters or numbers aren't printable

  return wParam;
}

static ImGuiMouseButton translateButton(const GdkEventButton *event)
{
  switch(event->button) {
  case 1: case 2: case 3: return event->button - 1;
  // 4/5/6/7 are scroll/thumb wheels, doesn't trigger WM_LBUTTON messages
  case 8: case 9:         return event->button - 5;
  default:                return ImGuiMouseButton_Left;
  }
}

std::optional<LRESULT> GDKWindow::handleMessage
  (const unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
  case WM_DROPFILES: {
    HDROP drop { reinterpret_cast<HDROP>(wParam) };
    m_ctx->beginDrag(drop);
    DragFinish(drop);
    m_ctx->endDrag(true);
    return 0;
  }
  case WM_SIZE:
    if(m_gl) {
      gdk_gl_context_make_current(m_gl);
      resizeTextures();
      gdk_gl_context_clear_current();
    }
    break; // continue handling in Window::proc
  case WM_LBUTTONDOWN: // for supporting thumb buttons
  case WM_LBUTTONUP: //   SWELL treats thumb buttons as Left
    if(auto *event { GdkEventHandler::currentEvent<GdkEventButton>(GDK_BUTTON_PRESS) })
      mouseDown(translateButton(event));
    else if(auto *event { GdkEventHandler::currentEvent<GdkEventButton>(GDK_BUTTON_RELEASE) })
      mouseUp(translateButton(event));
    else if(!GdkEventHandler::currentEvent<GdkEventButton>(GDK_2BUTTON_PRESS))
      break;  // do default SWELL message handling in Window
    return 0; // eat SWELL message if handled the GDK event
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
    keyEvent(wParam, lParam, msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
    return 0;
  case WM_PAINT:
    if(m_pixels)
      softwareBlit();
    return 0;
  }

  return std::nullopt;
}

static ImGuiKey translateGdkKey(const GdkEventKey *event)
{
  switch(event->keyval) {
  case GDK_KEY_Control_L: return ImGuiKey_LeftCtrl;
  case GDK_KEY_Control_R: return ImGuiKey_RightCtrl;
  case GDK_KEY_Shift_L:   return ImGuiKey_LeftShift;
  case GDK_KEY_Shift_R:   return ImGuiKey_RightShift;
  case GDK_KEY_Alt_L:     return ImGuiKey_LeftAlt;
  case GDK_KEY_Alt_R:     return ImGuiKey_RightAlt;
  case GDK_KEY_Super_L:   return ImGuiKey_LeftSuper;
  case GDK_KEY_Super_R:   return ImGuiKey_RightSuper;
  case GDK_KEY_KP_Enter:  return ImGuiKey_KeypadEnter;
  default:                return ImGuiKey_None;
  }
}

void GDKWindow::keyEvent(WPARAM swellKey, LPARAM lParam, const bool down)
{
  const GdkEventType expectedType { down ? GDK_KEY_PRESS : GDK_KEY_RELEASE };
  auto *gdkEvent { GdkEventHandler::currentEvent<GdkEventKey>(expectedType) };

  struct Modifier { unsigned int vkey; ImGuiKey modkey, ikey; };
  constexpr Modifier modifiers[] {
    { VK_CONTROL, ImGuiKey_ModCtrl,  ImGuiKey_LeftCtrl  },
    { VK_SHIFT,   ImGuiKey_ModShift, ImGuiKey_LeftShift },
    { VK_MENU,    ImGuiKey_ModAlt,   ImGuiKey_LeftAlt   },
    { VK_LWIN,    ImGuiKey_ModSuper, ImGuiKey_LeftSuper },
  };

  for(const auto &modifier : modifiers) {
    if(swellKey != modifier.vkey)
      continue;
    // post key events only when both sides of the modifier have the same state
    if(!!(GetAsyncKeyState(swellKey) & 0x8000) == down) {
      m_ctx->keyInput(modifier.modkey, down);
      if(!gdkEvent)
        m_ctx->keyInput(modifier.ikey, down);
    }
    break;
  }


  if(gdkEvent) {
    if(gtk_im_context_filter_keypress(m_ime, gdkEvent))
      return;

    if(ImGuiKey namedKey { translateGdkKey(gdkEvent) }) {
      m_ctx->keyInput(namedKey, down);
      return;
    }
  }
  else if(down)
    m_ctx->charInput(unmangleSwellChar(swellKey, lParam));

  if(swellKey < 256)
    m_ctx->keyInput(swellKey, down);
}
