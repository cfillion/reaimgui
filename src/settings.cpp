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

#include "settings.hpp"

#include "renderer.hpp"
#include "settings.rc.hpp"
#include "version.hpp"
#include "window.hpp"

#include <reaper_plugin.h>
#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h> // WDL_DLGRET

#ifdef _WIN32
#  include "win32_unicode.hpp"
#  define WIDEN(str)  widen(str).c_str()
#  define NARROW(str) narrow(str).c_str()
#else
#  define TCHAR char
#  define TEXT(str) str
#  define WIDEN(str) str
#  define NARROW(str) str
#endif

bool Settings::NoSavedSettings { false };
const RendererType *Settings::Renderer;

// constant values from REAPER
constexpr int WM_PREFS_APPLY  { WM_USER * 2 },
              IDC_PREFS_APPLY { 0x478 }, IDC_PREFS_HELP  { 0x4eb };

static void CALLBACK updateHelp(HWND hwnd, UINT, UINT_PTR, DWORD)
{
  static const TCHAR *shownText;

  if(!IsWindowVisible(hwnd)) {
    shownText = nullptr;
    return; // another preference page is active
  }

  constexpr const TCHAR *PREVIOUS_TEXT {};

  struct HelpText { int control; const TCHAR *text; };
  constexpr HelpText helpMap[] {
    { IDC_SAVEDSETTINGS, TEXT("Disable to force ReaImGui scripts to start with "
                              "their default first-use state (safe mode).") },
    { IDC_RENDERER,      TEXT("Select a different renderer if you encounter "
                              "compatibility problems.") },
    { IDC_RENDERERTXT,   PREVIOUS_TEXT },
  };

  POINT point;
  GetCursorPos(&point);

  const TCHAR *helpText;
  for(const HelpText &help : helpMap) {
    RECT rect;
    GetWindowRect(GetDlgItem(hwnd, help.control), &rect);
    if(help.text != PREVIOUS_TEXT)
      helpText = help.text;
    if(PtInRect(&rect, point)) {
      // repeatedly setting the same text flickers on Windows
      if(shownText != helpText) {
        SetDlgItemText(GetParent(hwnd), IDC_PREFS_HELP, helpText);
        shownText = helpText;
      }
      return;
    }
  }

  SetDlgItemText(GetParent(hwnd), IDC_PREFS_HELP, TEXT(""));
  shownText = nullptr;
}

static void fillRenderers(HWND combo)
{
  for(const RendererType *type : RendererType::knownTypes()) {
    const auto index {
      SendMessage(combo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(WIDEN(type->displayName)))
    };
    SendMessage(combo, CB_SETITEMDATA, index, reinterpret_cast<LPARAM>(type));
    if(type == Settings::Renderer)
      SendMessage(combo, CB_SETCURSEL, index, 0);
  }
}

static const RendererType *selectedRenderer(HWND combo)
{
  const auto index { SendMessage(combo, CB_GETCURSEL, 0, 0) };
  return reinterpret_cast<const RendererType *>
    (SendMessage(combo, CB_GETITEMDATA, index, 0));
}

static bool isChangeEvent(const short control, const short notification)
{
  switch(control) {
  case IDC_SAVEDSETTINGS:
    return true;
  case IDC_RENDERER:
    return notification == CBN_SELCHANGE;
  default:
    return false;
  }
}

static WDL_DLGRET settingsProc(HWND hwnd, const unsigned int message,
  const WPARAM wParam, const LPARAM lParam)
{
  switch(message) {
  case WM_INITDIALOG: {
    SetTimer(hwnd, 1, 200, &updateHelp); // same speed as REAPER

    CheckDlgButton(hwnd, IDC_SAVEDSETTINGS, !Settings::NoSavedSettings);
    fillRenderers(GetDlgItem(hwnd, IDC_RENDERER));

    return 1;
  }
  case WM_PREFS_APPLY:
    Settings::NoSavedSettings = !IsDlgButtonChecked(hwnd, IDC_SAVEDSETTINGS);
    Settings::Renderer = selectedRenderer(GetDlgItem(hwnd, IDC_RENDERER));
    Settings::save();
    return 0;
  case WM_SIZE: {
    RECT pageRect;
    GetClientRect(hwnd, &pageRect);
    const auto pageWidth  { pageRect.right - pageRect.left },
               pageHeight { pageRect.bottom - pageRect.top };
    SetWindowPos(GetDlgItem(hwnd, IDC_GROUPBOX), nullptr,
      0, 0, pageWidth, pageHeight,
      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
    return 0;
  }
  case WM_COMMAND:
    if(isChangeEvent(LOWORD(wParam), HIWORD(wParam)))
      EnableWindow(GetDlgItem(GetParent(hwnd), IDC_PREFS_APPLY), true);
    return 0;
  }

  return 0;
}

static HWND create(HWND parent)
{
  return CreateDialog(Window::s_instance, MAKEINTRESOURCE(IDD_SETTINGS),
    parent, &settingsProc);
}

static prefs_page_register_t g_page {
  "reaimgui", "ReaImGui", &create,
  0x9a, "", // parent page ID (Plug-ins, unknown parent idstr)
  0,        // won't have children
};

constexpr const TCHAR *APP { TEXT("reaimgui") },
                      *KEY_NOSAVEDSETTINGS { TEXT("nosavedsettings") },
                      *KEY_RENDERER        { TEXT("renderer_")
#if defined(_WIN32)
                                             L"win32"
#elif defined(__APPLE__)
                                             "cocoa"
#else
                                             "gdk"
#endif
                                           };

void Settings::setup()
{
  plugin_register("prefpage", reinterpret_cast<void *>(&g_page));

#ifdef _WIN32
  const std::wstring &fn { widen(get_ini_file()) };
  const wchar_t *file { fn.c_str() };
#else
  const char *file { get_ini_file() };
#endif

  TCHAR buffer[4096];
  const DWORD bufSize { static_cast<DWORD>(std::size(buffer)) };

  NoSavedSettings = GetPrivateProfileInt
    (APP, KEY_NOSAVEDSETTINGS, NoSavedSettings, file);
  GetPrivateProfileString(APP, KEY_RENDERER, TEXT(""), buffer, bufSize, file);
  Renderer = RendererType::bestMatch(NARROW(buffer));

  // store default settings without waiting for the user to apply
  save();
}

void Settings::save()
{
#ifdef _WIN32
  const std::wstring &fn { widen(get_ini_file()) };
  const wchar_t *file { fn.c_str() };
#else
  const char *file { get_ini_file() };
#endif

  constexpr const TCHAR *bools[] { TEXT("0"), TEXT("1") };
  WritePrivateProfileString(APP, KEY_NOSAVEDSETTINGS, bools[NoSavedSettings], file);
  WritePrivateProfileString(APP, KEY_RENDERER, WIDEN(Renderer->id), file);
}

void Settings::teardown()
{
  plugin_register("-prefpage", reinterpret_cast<void *>(&g_page));
}

#ifndef _WIN32
#  include <swell/swell-dlggen.h>
#  include "settings.rc_mac_dlg"
#  include <swell/swell-menugen.h>
#  include "settings.rc_mac_menu"
#endif
