/* ReaImGui: ReaScript binding for dear imgui
 * Copyright (C) 2021  Christian Fillion
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

#include "api_helper.hpp"

#include <array>
#include <cassert>
#include <reaper_colortheme.h>
#include <reaper_plugin_functions.h>
#include <regex>
#include <sstream>
#include <vector>

#ifdef UNICODE
#  undef FindWindowEx
#  define FindWindowEx FindWindowExA
#  undef GetWindowText
#  define GetWindowText GetWindowTextA
#endif

// here lies quick & dirty code...

constexpr int LABEL_SIZE { 255 };
constexpr int HIGHLIGHTED { 0xFD78FD }, RGB_MASK { 0xFFFFFF };

static std::vector<std::array<char, LABEL_SIZE>> labels;
static bool scan {};
static int step {};
static LRESULT steps {};
static int colorCount {};
static int scrollTo { -1 };
static int *colors {};
static std::string cppStruct;

void scanStep()
{
  constexpr int IDC_COLOR { 0x3e9 };

  HWND finder { FindWindowEx(nullptr, nullptr, nullptr, "Theme element finder") };
  if(!finder)
    return;

  HWND colorBox = GetDlgItem(finder, IDC_COLOR);
  assert(colorBox);

  steps = SendMessage(colorBox, CB_GETCOUNT, 0, 0);

  if(step >= steps) {
    DestroyWindow(finder);
    scan = false;
    return;
  }

  SendMessage(colorBox, CB_SETCURSEL, step++, 0);
  SendMessage(finder, WM_COMMAND, (CBN_SELCHANGE<<16) | IDC_COLOR, 0);

  int matches { 0 };
  for(int i = 0; i < colorCount; ++i) {
    if((colors[i] & RGB_MASK) == HIGHLIGHTED) {
      GetWindowText(colorBox, labels[i].data(), labels[i].size());
      size_t len { strlen(labels[i].data()) };
      if(matches++ > 0 && len < labels[i].size() - strlen("_copy0")) {
        labels[i][len++] = '_', labels[i][len++] = 'c', labels[i][len++] = 'o',
        labels[i][len++] = 'p', labels[i][len++] = 'y',
        labels[i][len++] = '0' + (matches - 2);
      }
      scrollTo = i;
    }
  }
}

static void formatFieldName(std::string &field)
{
  std::transform(field.begin(), field.end(), field.begin(),
    [](const char c) { return std::tolower(c); });

  size_t pos;
  if((pos = field.find('<')) != std::string::npos)
    field.insert(pos, "_lt");
  if((pos = field.find('>')) != std::string::npos)
    field.insert(pos, "_gt");

  static std::regex removeRegex { "^[^a-z0-9]+|[^a-z0-9]+$" };
  field = std::regex_replace(field, removeRegex, "");

  static std::regex underscoreRegex { "[^a-z0-9]+" };
  field = std::regex_replace(field, underscoreRegex, "_");
}

static bool isArrayField(const std::string &field,
  const std::string *expectedPrefix, const int expectedIndex,
  std::string *prefix = nullptr)
{
  static std::regex arrayRegex { "^(.+)_([0-9]+)$" };
  std::smatch matches;

  if(!std::regex_search(field, matches, arrayRegex))
    return false;

  if(expectedPrefix && matches[1].str() != *expectedPrefix)
    return false;

  if(std::stoi(matches[2].str()) != expectedIndex)
    return false;

  if(prefix)
    *prefix = matches[1].str();

  return true;
}

static void generateStruct()
{
  std::ostringstream out;
  int offsetFromLast {}, unknowns {}, arraySize {};
  std::string arrayField;

  out << "#ifndef REAPER_COLORTHEME_H\n";
  out << "#define REAPER_COLORTHEME_H\n\n";

  out << "struct ColorTheme {\n";

  for(int i = 0; i < colorCount; ++i) {
    if(!labels[i][0]) {
      ++offsetFromLast;
      continue;
    }

    if(offsetFromLast > 0) {
      if(arraySize > 0) {
        out << '[' << arraySize << "];\n";
        arraySize = 0;
      }

      out << "  int _unknown" << unknowns++;
      if(offsetFromLast > 1)
        out << '[' << offsetFromLast << ']';
      out << ";\n";
      offsetFromLast = 0;
    }

    std::string field { labels[i].data() };
    formatFieldName(field);

    if(arraySize == 0) {
      out << "  int ";
      if(isArrayField(field, nullptr, 1, &arrayField)) {
        out << arrayField;
        arraySize = 1;
        continue;
      }
      else
        out << field;
    }
    else if(isArrayField(field, &arrayField, arraySize + 1)) {
      ++arraySize;
      continue;
    }
    else {
      out << '[' << arraySize << "];\n";
      arraySize = 0;

      out << "  int " << field;
    }

    out << ";\n";
  }

  if(arraySize > 0)
    out << '[' << arraySize << "];\n";

  out << "};\n\n#endif\n";

  cppStruct = out.str();
}

void showStruct()
{
  constexpr float PAD { 20.f };
  const ImVec2 &dspSize { ImGui::GetIO().DisplaySize };
  ImGui::SetNextWindowPos({PAD, PAD}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({dspSize.x - (PAD*2), dspSize.y - (PAD*2)}, ImGuiCond_FirstUseEver);

  bool open { true };
  if(!ImGui::BeginPopupModal("Memory layout (C++)###export", &open))
    return;

  if(ImGui::Button("Print to standard output"))
    printf("%s", cppStruct.c_str());
  ImGui::InputTextMultiline("##struct", cppStruct.data(), cppStruct.size(), {-FLT_MIN, -FLT_MIN}, ImGuiInputTextFlags_ReadOnly);
  ImGui::EndPopup();
}

static void loadTheme()
{
  int themeSize;
  ColorTheme *theme { reinterpret_cast<ColorTheme *>(GetColorThemeStruct(&themeSize)) };
  static int offset { 0 };
  themeSize -= offset;
  colorCount = themeSize / 4;

  char *bytes { reinterpret_cast<char *>(theme) + offset };
  colors = reinterpret_cast<int *>(bytes);

  labels.resize(colorCount);
}

static void inspectTheme()
{
  loadTheme();

  if(scan)
    scanStep();

  constexpr ImGuiWindowFlags window_flags {
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoMove
  };

  ImGui::SetNextWindowPos(ImVec2{0.f, 0.f});
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::Begin("Color theme inspector", nullptr, window_flags);

  // ImGui::InputInt("struct offset", &offset, 1, 4, ImGuiInputTextFlags_CharsHexadecimal);

  if(ImGui::Button("Export")) {
    generateStruct();
    ImGui::OpenPopup("###export");
  }
  ImGui::SameLine();
  if(ImGui::Button("Clear")) {
    for(auto &arr : labels)
      arr[0] = '\0';
    scrollTo = 0;
  }
  ImGui::SameLine();
  if(ImGui::Button(scan ? "Stop" : "Auto-detect", ImVec2{100.f, 0.f})) {
    if(!scan) {
      SendMessage(GetMainHwnd(), WM_COMMAND, 40690, 0);
      if(step == steps)
        step = 0;
    }
    scan = !scan;
  }
  ImGui::SameLine();
  ImGui::ProgressBar(static_cast<float>(step) / steps);

  constexpr ImGuiTableFlags table_flags {
    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Borders |
    ImGuiTableFlags_ScrollY
  };

  if(ImGui::BeginTable("colors", 4, table_flags)) {
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Idx", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    for(int i = 0; i < colorCount; ++i) {
      ImGui::TableNextRow();
      ImGui::PushID(i);

      if(scrollTo == i) {
        ImGui::SetScrollHereY();
        scrollTo = -1;
      }

      bool is_highlighted { (colors[i] & RGB_MASK) == HIGHLIGHTED };
      const Color &color { Color::fromTheme(colors[i]) };

      bool found { labels[i][0] != '\0' };
      ImGui::TableNextColumn();
      if(ImGui::Checkbox("##known", &found))
        labels[i][0] = '\0';

      ImGui::TableNextColumn();
      char id[32];
      snprintf(id, sizeof(id), "%04d\n", i);
      if(is_highlighted)
        ImGui::Selectable(id, true);
      else
        ImGui::TextUnformatted(id);

      ImGui::TableNextColumn();
      ImGui::ColorButton(labels[i][0] ? labels[i].data() : "unknown", color);
      ImGui::SameLine();
      ImGui::Text("0x%08X (#%08X, extra=%02X)",
        colors[i], color.pack(), colors[i] >> 24 & 0xFF);

      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputText("##label", labels[i].data(), LABEL_SIZE);

      ImGui::PopID();
    }
    ImGui::EndTable();
  }

  showStruct();
  ImGui::End();
}

DEFINE_API(void, InspectTheme, (ImGui_Context*,ctx), "",
{
  FRAME_GUARD;
  inspectTheme();
});
