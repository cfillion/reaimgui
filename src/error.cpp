/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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

#include "error.hpp"

#include "context.hpp"
#include "dialog.hpp"
#include "platform.hpp"
#include "settings.hpp"
#include "win32_unicode.hpp"
#include "window.hpp"

#include <reaper_plugin_functions.h>
#include <vector>
#include <WDL/wdltypes.h> // WDL_DLGRET

struct ErrorReport {
  std::string title;
  std::runtime_error ex;
  const TCHAR *header, *message;
};

class ErrorReporter {
public:
  static void report(const ErrorReport &&);

  const ErrorReport *current() const { return &m_errors[m_current]; }
  size_t index() const { return m_current; }
  size_t size()  const { return m_errors.size(); }

  void setIndex(size_t);
  void prev() { setIndex(index() - 1); }
  void next() { setIndex(index() + 1); }

private:
  ErrorReporter();

  HWND m_window;
  std::vector<ErrorReport> m_errors;
  size_t m_current;
};

static std::unique_ptr<ErrorReporter> g_reporter;

constexpr const TCHAR *HEADER_FONT_FAMILY
#ifdef _WIN32
  {L"MS Shell Dlg 2"};
#else
  {"Arial"};
#endif

constexpr int HEADER_FONT_SIZE
#ifdef __APPLE__
  {13};
#else
  {15};
#endif

static RECT calcHeaderRect(HWND hwnd)
{
  RECT rect;
  GetWindowRect(GetDlgItem(hwnd, IDC_MESSAGE), &rect);
  ScreenToClient(hwnd, reinterpret_cast<POINT *>(&rect));
  ScreenToClient(hwnd, reinterpret_cast<POINT *>(&rect) + 1);
  rect.top = rect.left;
  return rect;
}

static WDL_DLGRET errorProc(HWND hwnd, const unsigned int msg,
  const WPARAM wParam, const LPARAM)
{
  switch(msg) {
  case WM_INITDIALOG:
    SetFocus(GetDlgItem(hwnd, IDOK));
    return 0;
  case WM_USER: {
    const ErrorReport *err {g_reporter->current()};

    char title[128];
    snprintf(title, sizeof(title), "%s [%zu/%zu]",
      err->title.c_str(), g_reporter->index() + 1, g_reporter->size());
    SetWindowText(hwnd, WIDEN(title));

    SetDlgItemText(hwnd, IDC_MESSAGE, err->message);
    SetDlgItemText(hwnd, IDC_REPORT, WIDEN(err->ex.what()));

    // update the bold header text
    RECT headerRect {calcHeaderRect(hwnd)};
    InvalidateRect(hwnd, &headerRect, true);

    EnableWindow(GetDlgItem(hwnd, IDC_PREV), g_reporter->index() > 0);
    EnableWindow(GetDlgItem(hwnd, IDC_NEXT),
      g_reporter->index() + 1 < g_reporter->size());

    ShowWindow(hwnd, SW_SHOW);
    return 0;
  }
  case WM_PAINT: {
    const ErrorReport *err {g_reporter->current()};
#ifdef __APPLE__
    constexpr float scale {1.f};
#else
    const float scale {Platform::scaleForWindow(hwnd)};
#endif
    PAINTSTRUCT ps;
    HDC dc {BeginPaint(hwnd, &ps)};

    HFONT headerFont {CreateFont(HEADER_FONT_SIZE * scale, 0, 0, 0, FW_BOLD,
      false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, HEADER_FONT_FAMILY)};
    RECT headerRect {calcHeaderRect(hwnd)};
    HGDIOBJ defaultFont {SelectObject(dc, headerFont)};
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, GetSysColor(COLOR_BTNTEXT));
    DrawText(dc, err->header, -1, &headerRect, DT_LEFT);
    SelectObject(dc, defaultFont);
    DeleteObject(headerFont);

    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_COMMAND:
    switch(LOWORD(wParam)) {
    case IDC_SETTINGS:
      Settings::open();
      break;
    case IDC_PREV:
      g_reporter->prev();
      break;
    case IDC_NEXT:
      g_reporter->next();
      break;
    case IDOK:
    case IDCANCEL:
      DestroyWindow(hwnd);
      break;
    }
    return 0;
  case WM_KEYDOWN:
    switch(wParam) {
    case VK_LEFT:
      g_reporter->prev();
      break;
    case VK_RIGHT:
      g_reporter->next();
      break;
    }
    return 0;
  case WM_DESTROY:
    g_reporter.reset();
    return 0;
  }

  return 0;
}

void ErrorReporter::report(const ErrorReport &&e)
{
  if(!g_reporter)
    g_reporter.reset(new ErrorReporter);

  g_reporter->m_errors.emplace_back(e);
  g_reporter->setIndex(g_reporter->size() - 1);
}

ErrorReporter::ErrorReporter()
  : m_current {static_cast<size_t>(-1)}
{
  m_window = CreateDialog(Window::s_instance,
    MAKEINTRESOURCE(IDD_ERROR), GetMainHwnd(), errorProc);
}

void ErrorReporter::setIndex(const size_t index)
{
  if(index >= size())
    return;

  m_current = index;
  SendMessage(g_reporter->m_window, WM_USER, 0, 0);
}

#define PREF_PATH TEXT("Preferences > Plug-ins > ReaImGui")

void Error::report(Context *ctx, const imgui_error &ex)
{
  ErrorReporter::report({
    ctx->name(), ex,
    TEXT("ImGui assertion failed"),
    TEXT("The ImGui context was left in an invalid state and cannot continue.\n")
    TEXT("Report this problem along with the error description below to the script")
    TEXT(" or plugin developer.\n(Disable state restoration in ") PREF_PATH
    TEXT(" to reset saved settings.)"),
  });
}

void Error::report(Context *ctx, const backend_error &ex)
{
  ErrorReporter::report({
    ctx->name(), ex,
    TEXT("ReaImGui backend error"),
    TEXT("The current graphics rendering engine may not be compatible with your")
    TEXT(" computer.\nTry selecting a different graphics renderer in ") PREF_PATH
    TEXT(" or report this problem if the issue persists."),
  });
}

void Error::imguiAssertionFailure(const char *message)
{
  throw imgui_error {message};
}

void Error::imguiDebugBreak()
{
  throw reascript_error {"debug break"};
}
