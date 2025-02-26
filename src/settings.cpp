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

#define SETTINGS_IMPLEMENT
#include "settings.hpp"

#include "action.hpp"
#include "dialog.hpp"
#include "renderer.hpp"
#include "win32_unicode.hpp"
#include "window.hpp"

#include <algorithm>
#include <optional>
#include <reaper_plugin.h>
#include <reaper_plugin_functions.h>
#include <variant>
#include <WDL/wdltypes.h> // WDL_DLGRET

template<typename T>
struct Setting;

struct Checkbox {
  enum Flags { None = 0, Invert = 1<<0, NoAction = 1<<1 };
  int box, flags = None;

  void setup(const Setting<bool> &) const;
  void setLabel(HWND, const TCHAR *) const;
  void setValue(HWND, bool) const;
  void apply(HWND, bool *)  const;
  bool hitTest(HWND, POINT) const;
  std::optional<bool> isChangeEvent(short control, short notification) const;
};

struct Combobox {
  int box, label;

  void setup(const Setting<const RendererType *> &) const {};
  void setLabel(HWND, const TCHAR *)        const;
  void setValue(HWND, const RendererType *) const;
  void apply(HWND, const RendererType **)   const;
  bool hitTest(HWND, POINT) const;
  std::optional<bool> isChangeEvent(short control, short notification) const;
};

template<typename T> struct Control;
template<> struct Control<bool>                 { using type = Checkbox; };
template<> struct Control<const RendererType *> { using type = Combobox; };

template<typename T>
struct Setting {
  T *value;
  T defaultValue;
  const TCHAR *key, *label, *help;
  typename Control<T>::type control;

  void read(const TCHAR *file)  const;
  void write(const TCHAR *file) const;
  void reset() const { *value = defaultValue; }
};

template<typename... Ts>
struct SettingVariant : std::variant<Setting<Ts>...> {
  template<typename T, typename... Args>
  constexpr SettingVariant(T *value, Args&&... args)
    : std::variant<Setting<Ts>...> {Setting<T> {value, args...}} {}
};

#if defined(_WIN32)
#  define PLATFORM_SUFFIX L"_win32"
#elif defined(__APPLE__)
#  define PLATFORM_SUFFIX "_cocoa"
#else
#  define PLATFORM_SUFFIX "_gdk"
#endif

constexpr SettingVariant<bool, const RendererType *> SETTINGS[] {
  {&Settings::NoSavedSettings, false, TEXT("nosavedsettings"),
   TEXT("Restore window position, size, dock state and table settings"),
   TEXT("Disable to force ReaImGui scripts to start with "
        "their default first-use state (safe mode)."),
   Checkbox {IDC_SAVEDSETTINGS, Checkbox::Invert},
  },
  {&Settings::DockingEnable, true, TEXT("dockingenable"),
   TEXT("Enable docking by default"),
   TEXT("Drag the titlebar to dock windows into REAPER dockers or into other "
        "windows of the same script instance."),
   Checkbox {IDC_DOCKINGENABLE},
  },
  {&Settings::DockingNoSplit, false, TEXT("dockingnosplit"),
   TEXT("Enable window splitting when docking"),
   TEXT("Disable to limit docking to merging multiple windows together into "
        "tab bars (simplified docking mode)."),
   Checkbox {IDC_DOCKSPLIT, Checkbox::Invert},
  },
  {&Settings::DockingWithShift, false, TEXT("dockingwithshift"),
   TEXT("Dock only when holding Shift"),
   TEXT("Press the Shift key to disable or enable docking when dragging "
        "windows using the title bar. This option inverts the behavior."),
   Checkbox {IDC_DOCKWITHSHIFT},
  },
  {&Settings::DockingTransparentPayload, false, TEXT("dockingtransparentpayload"),
   TEXT("Make windows transparent when docking"),
   TEXT("Windows become semi-transparent when docking into another window. "
        "Docking boxes are shown only in the target window."),
   Checkbox {IDC_DOCKTRANSPARENT},
  },
  {&Settings::Renderer, nullptr, TEXT("renderer") PLATFORM_SUFFIX,
   TEXT("Graphics renderer (advanced):"),
   TEXT("Select a different renderer if you encounter compatibility problems."),
   Combobox {IDC_RENDERER, IDC_RENDERERTXT},
  },
  {&Settings::ForceSoftware, false, TEXT("forcecpu") PLATFORM_SUFFIX,
   TEXT("Disable hardware acceleration"),
   TEXT("Enable this option force the use of software rendering. May improve "
        "compatibility at the cost of potentially higher CPU usage."),
   Checkbox {IDC_FORCESOFTWARE, Checkbox::NoAction}
  },
};

constexpr const TCHAR *SECTION {TEXT("reaimgui")};

template<>
void Setting<bool>::read(const TCHAR *file) const
{
  *value = GetPrivateProfileInt(SECTION, key, defaultValue, file);
}

template<>
void Setting<const RendererType *>::read(const TCHAR *file) const
{
  TCHAR id[32];
  GetPrivateProfileString(SECTION, key, TEXT(""), id, std::size(id), file);
  *value = RendererType::bestMatch(NARROW(id));
}

template<>
void Setting<bool>::write(const TCHAR *file) const
{
  constexpr const TCHAR *bools[] {TEXT("0"), TEXT("1")};
  WritePrivateProfileString(SECTION, key, bools[*value], file);
}

template<>
void Setting<const RendererType *>::write(const TCHAR *file) const
{
  WritePrivateProfileString(SECTION, key, WIDEN((*value)->id), file);
}

template<>
void Setting<const RendererType *>::reset() const
{
  *value = RendererType::bestMatch("");
}

void Checkbox::setLabel(HWND window, const TCHAR *text) const
{
  SetDlgItemText(window, box, text);
}

void Combobox::setLabel(HWND window, const TCHAR *text) const
{
  SetDlgItemText(window, label, text);
}

void Checkbox::setValue(HWND window, const bool value) const
{
  const bool invert {(flags & Invert) != 0};
  CheckDlgButton(window, box, value ^ invert);
}

void Combobox::setValue(HWND window, const RendererType *value) const
{
  HWND combo {GetDlgItem(window, box)};
  SendMessage(combo, CB_RESETCONTENT, 0, 0);
  for(const RendererType *type {RendererType::head()}; type; type = type->next) {
    const auto index {
      SendMessage(combo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(WIDEN(type->displayName)))
    };
    SendMessage(combo, CB_SETITEMDATA, index, reinterpret_cast<LPARAM>(type));
    if(type == value)
      SendMessage(combo, CB_SETCURSEL, index, 0);
  }
}

void Checkbox::apply(HWND window, bool *value) const
{
  const bool invert {(flags & Invert) != 0};
  *value = !!IsDlgButtonChecked(window, box) ^ invert;
}

void Combobox::apply(HWND window, const RendererType **value) const
{
  HWND combo {GetDlgItem(window, box)};
  const auto index {SendMessage(combo, CB_GETCURSEL, 0, 0)};
  *value = reinterpret_cast<const RendererType *>
    (SendMessage(combo, CB_GETITEMDATA, index, 0));
}

static bool hitTest(HWND window, const POINT point, const int control)
{
  RECT rect;
  GetWindowRect(GetDlgItem(window, control), &rect);
  return PtInRect(&rect, point);
}

bool Checkbox::hitTest(HWND window, const POINT point) const
{
  return ::hitTest(window, point, box);
}

bool Combobox::hitTest(HWND window, const POINT point) const
{
  return ::hitTest(window, point, box) || ::hitTest(window, point, label);
}

std::optional<bool> Checkbox::isChangeEvent
  (const short controlId, const short notification) const
{
  if(controlId == box)
    return notification == BN_CLICKED;
  return std::nullopt;
}

std::optional<bool> Combobox::isChangeEvent
  (const short controlId, const short notification) const
{
  if(controlId == box)
    return notification == CBN_SELCHANGE;
  return std::nullopt;
}

static std::string makeActionName(const TCHAR *key)
{
  std::string name {narrow(key)};
  std::transform(name.begin(), name.end(), name.begin(), toupper);
  return name;
}

void Checkbox::setup(const Setting<bool> &setting) const
{
  if(flags & NoAction)
    return;

  const bool invert {(flags & Invert) != 0};
  new Action {
    makeActionName(setting.key), narrow(setting.label),
    [value = setting.value        ] { *value = !*value;       },
    [value = setting.value, invert] { return *value ^ invert; },
  };
}

static void updateHelp(HWND hwnd)
{
  constexpr int IDC_PREFS_HELP {0x4eb}, IDT_PREFS_HELP_CLEAR {0x654};

  static const TCHAR *shownText;

  if(!IsWindowVisible(hwnd)) {
    shownText = nullptr;
    return; // another preference page is active
  }

  HWND prefs {GetParent(hwnd)};
  KillTimer(prefs, IDT_PREFS_HELP_CLEAR);

  POINT point;
  GetCursorPos(&point);

  for(const auto &setting : SETTINGS) {
    if(std::visit([hwnd, point](const auto &setting) {
      if(!setting.control.hitTest(hwnd, point))
        return false;
      if(shownText == setting.help)
        return true; // repeatedly setting the same text flickers on Windows
      shownText = setting.help;
      SetDlgItemText(GetParent(hwnd), IDC_PREFS_HELP, setting.help);
      return true;
    }, setting))
      return;
  }

  shownText = nullptr;
  SetTimer(prefs, IDT_PREFS_HELP_CLEAR, 200, nullptr); // same speed as REAPER
}

static bool isChangeEvent(const short control, const short notification)
{
  for(const auto &setting : SETTINGS) {
    if(auto rv {std::visit([control, notification] (const auto &setting) {
      return setting.control.isChangeEvent(control, notification);
    }, setting)})
      return *rv;
  }

  return false;
}

template<typename T>
static const Setting<T> *findSetting(T *value)
{
  for(const auto &variant : SETTINGS) {
    const auto setting {std::get_if<Setting<T>>(&variant)};
    if(setting && setting->value == value)
      return setting;
  }

  return nullptr;
}

static void updateRendererOptions(HWND hwnd)
{
  static auto renderer      {findSetting(&Settings::Renderer)};
  static auto forceSoftware {findSetting(&Settings::ForceSoftware)};

  const RendererType *type;
  renderer->control.apply(hwnd, &type);

  EnableWindow(GetDlgItem(hwnd, forceSoftware->control.box),
    type->flags & RendererType::CanForceSoftware);
}

static void resetDefaults(HWND hwnd)
{
  if(IDOK != MessageBox(hwnd,
      TEXT("Are you sure you want to restore the default settings?"),
      TEXT("ReaImGui settings"),
      MB_OKCANCEL | MB_ICONWARNING))
    return;

  for(const auto &setting : SETTINGS) {
    std::visit([hwnd] (const auto &setting) {
      setting.reset();
      setting.control.setValue(hwnd, *setting.value);
    }, setting);
  }

  Settings::save();
  Action::refreshAll();
  updateRendererOptions(hwnd);
}

static void processCommand(HWND hwnd,
  const short control, const short notification)
{
  if(isChangeEvent(control, notification)) {
    constexpr int IDC_PREFS_APPLY {0x478};
    EnableWindow(GetDlgItem(GetParent(hwnd), IDC_PREFS_APPLY), true);
    if(control == IDC_RENDERER)
      updateRendererOptions(hwnd);
    return;
  }

  switch(control) {
  case IDC_RESETDEFAULTS:
    resetDefaults(hwnd);
    break;
  }
}

static WDL_DLGRET settingsProc(HWND hwnd, const unsigned int message,
  const WPARAM wParam, const LPARAM)
{
  constexpr int WM_PREFS_APPLY {WM_USER * 2};

  switch(message) {
  case WM_INITDIALOG: {
    for(const auto &setting : SETTINGS) {
      std::visit([hwnd] (const auto &setting) {
        setting.control.setLabel(hwnd, setting.label);
        setting.control.setValue(hwnd, *setting.value);
      }, setting);
    }
    updateRendererOptions(hwnd);
    EnableWindow(GetDlgItem(hwnd, IDC_VERSION), false);
    return 0; // don't focus the first control
  }
  case WM_SIZE: {
    RECT pageRect;
    GetClientRect(hwnd, &pageRect);
    const auto pageWidth  {pageRect.right - pageRect.left},
               pageHeight {pageRect.bottom - pageRect.top};
    SetWindowPos(GetDlgItem(hwnd, IDC_GROUPBOX), nullptr,
      0, 0, pageWidth, pageHeight,
      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
    return 0;
  }
  case WM_SHOWWINDOW: // to clear shownText when the page becomes inactive
  case WM_SETCURSOR:  // WM_MOUSEMOVE is not sent over children controls (win32)
    updateHelp(hwnd);
    return 0;
  case WM_COMMAND:
    processCommand(hwnd, LOWORD(wParam), HIWORD(wParam));
    return 0;
  case WM_PREFS_APPLY:
    for(const auto &setting : SETTINGS) {
      std::visit([hwnd] (const auto &setting) {
        setting.control.apply(hwnd, setting.value);
      }, setting);
    }
    Settings::save();
    Action::refreshAll();
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

void Settings::open()
{
  ViewPrefs(0, g_page.idstr);
}

void Settings::setup()
{
  plugin_register("prefpage", reinterpret_cast<void *>(&g_page));

#ifdef _WIN32
  const std::wstring &fn {widen(get_ini_file())};
  const wchar_t *file {fn.c_str()};
#else
  const char *file {get_ini_file()};
#endif

  for(const auto &setting : SETTINGS) {
    std::visit([file] (const auto &setting) {
      setting.read(file);
      setting.control.setup(setting);

      // store default settings without waiting for the user to apply
      setting.write(file);
    }, setting);
  }
}

void Settings::save()
{
#ifdef _WIN32
  const std::wstring &fn {widen(get_ini_file())};
  const wchar_t *file {fn.c_str()};
#else
  const char *file {get_ini_file()};
#endif

  for(const auto &setting : SETTINGS)
    std::visit([file] (const auto &setting) { setting.write(file); }, setting);
}

void Settings::teardown()
{
  plugin_register("-prefpage", reinterpret_cast<void *>(&g_page));
}
