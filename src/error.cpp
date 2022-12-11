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

#include "error.hpp"

#include "context.hpp"
#include "dialog.hpp"
#include "platform.hpp"
#include "settings.hpp"
#include "win32_unicode.hpp"
#include "window.hpp"

#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h> // WDL_DLGRET

struct ErrorReport {
  std::string title;
  std::runtime_error ex;
  const TCHAR *header, *message;
};

constexpr const TCHAR *HEADER_FONT_FAMILY
#ifdef _WIN32
  { L"MS Shell Dlg 2" };
#else
  { "Arial" };
#endif

constexpr int HEADER_FONT_SIZE
#ifdef __APPLE__
  { 13 };
#else
  { 15 };
#endif

static WDL_DLGRET errorProc(HWND hwnd, const unsigned int msg,
  const WPARAM wParam, const LPARAM lParam)
{
  ErrorReport *err { reinterpret_cast<ErrorReport *>
    (msg == WM_INITDIALOG ? lParam : GetWindowLongPtr(hwnd, GWLP_USERDATA)) };

  switch(msg) {
  case WM_INITDIALOG:
    SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
    SetWindowText(hwnd, WIDEN(err->title.c_str()));
    SetDlgItemText(hwnd, IDC_MESSAGE, err->message);
    SetDlgItemText(hwnd, IDC_REPORT, WIDEN(err->ex.what()));


    SetFocus(GetDlgItem(hwnd, IDOK));
    ShowWindow(hwnd, SW_SHOW);
    return 0;
  case WM_COMMAND:
    switch(LOWORD(wParam)) {
    case IDC_SETTINGS:
      Settings::open();
      break;
    case IDOK:
    case IDCANCEL:
      DestroyWindow(hwnd);
      break;
    }
    return 0;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC dc { BeginPaint(hwnd, &ps) };

#ifdef __APPLE__
    constexpr float scale { 1.f };
#else
    const float scale { Platform::scaleForWindow(hwnd) };
#endif
    HFONT headerFont { CreateFont(HEADER_FONT_SIZE * scale, 0, 0, 0, FW_BOLD,
      false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, HEADER_FONT_FAMILY) };
    RECT headerRect;
    GetWindowRect(GetDlgItem(hwnd, IDC_MESSAGE), &headerRect);
    ScreenToClient(hwnd, reinterpret_cast<POINT *>(&headerRect));
    ScreenToClient(hwnd, reinterpret_cast<POINT *>(&headerRect) + 1);
    headerRect.top = headerRect.left;

    HGDIOBJ defaultFont { SelectObject(dc, headerFont) };
    SetTextColor(dc, GetSysColor(COLOR_BTNTEXT));
    DrawText(dc, err->header, -1, &headerRect, DT_LEFT);
    SelectObject(dc, defaultFont);
    DeleteObject(headerFont);

    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_DESTROY:
    delete err;
    return 0;
  }

  return 0;
}

static void showError(const ErrorReport *e)
{
  CreateDialogParam(Window::s_instance, MAKEINTRESOURCE(IDD_ERROR),
    GetMainHwnd(), errorProc, reinterpret_cast<LPARAM>(e));
}

#define PREF_PATH TEXT("Preferences > Plug-ins > ReaImGui")

void Error::report(Context *ctx, const imgui_error &ex)
{
  showError(new ErrorReport {
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
  showError(new ErrorReport {
    ctx->name(), ex,
    TEXT("ReaImGui backend error"),
    TEXT("The current graphics rendering engine may not be compatible with your")
    TEXT(" computer.\nTry selecting a different graphics renderer in ") PREF_PATH
    TEXT(" or report this problem if the issue persists."),
  });
}
