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

#include "settings.rc.hpp"
#include "version.hpp"
#include "window.hpp"

#include <reaper_plugin.h>
#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h> // WDL_DLGRET

#ifdef _WIN32
#  include "win32_unicode.hpp"
#else
#  define TEXT(str) str
#  define TCHAR char
#endif

bool Settings::NoSavedSettings { false };

constexpr int WM_PREFS_APPLY  { WM_USER * 2 },
              IDC_PREFS_APPLY { 0x478 }, IDC_PREFS_HELP  { 0x4eb };

static void CALLBACK updateHelp(HWND hwnd, UINT, UINT_PTR, DWORD)
{
  if(!IsWindowVisible(hwnd))
    return; // another preference page is active

  struct HelpText { int control; const TCHAR *text; };
  constexpr HelpText helpMap[] {
    { IDC_SAVEDSETTINGS,
      TEXT("Disable to force ReaImGui scripts to start with their default first-use state (safe mode).") },
  };

  POINT point;
  GetCursorPos(&point);

  for(const HelpText &help : helpMap) {
    RECT rect;
    GetWindowRect(GetDlgItem(hwnd, help.control), &rect);
    if(PtInRect(&rect, point)) {
      SetDlgItemText(GetParent(hwnd), IDC_PREFS_HELP, help.text);
      return;
    }
  }

  SetDlgItemText(GetParent(hwnd), IDC_PREFS_HELP, TEXT(""));
}

static WDL_DLGRET settingsProc(HWND hwnd, const unsigned int message,
  const WPARAM wParam, const LPARAM lParam)
{
  switch(message) {
  case WM_INITDIALOG:
    SetTimer(hwnd, 1, 200, &updateHelp); // same speed as REAPER
    CheckDlgButton(hwnd, IDC_SAVEDSETTINGS, !Settings::NoSavedSettings);
    return 1;
  case WM_COMMAND:
    // enable the Apply button
    EnableWindow(GetDlgItem(GetParent(hwnd), IDC_PREFS_APPLY), true);
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
  case WM_PREFS_APPLY:
    Settings::NoSavedSettings = !IsDlgButtonChecked(hwnd, IDC_SAVEDSETTINGS);
    Settings::save();
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
                      *KEY_NOSAVEDSETTINGS { TEXT("nosavedsettings") };

void Settings::setup()
{
  plugin_register("prefpage", reinterpret_cast<void *>(&g_page));

#ifdef _WIN32
  const std::wstring &fn { widen(get_ini_file()) };
  const wchar_t *file { fn.c_str() };
#else
  const char *file { get_ini_file() };
#endif

  NoSavedSettings = GetPrivateProfileInt
    (APP, KEY_NOSAVEDSETTINGS, NoSavedSettings, file);
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
  WritePrivateProfileString(APP, KEY_NOSAVEDSETTINGS,
    bools[Settings::NoSavedSettings], file);
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
