#include "profiler.hpp"

#include "context.hpp"
#include "color.hpp"

#include <algorithm>
#include <imgui/imgui.h>
#include <math.h>
#include <reaper_plugin_functions.h>

Profiler *g_prof;

Profiler::Profiler()
  : m_sortCol {}, m_sortAsc {}, m_showApi {}
{
  plugin_register("timer", reinterpret_cast<void *>(&timerTick));
  g_prof = this;
}

Profiler::~Profiler()
{
  plugin_register("-timer", reinterpret_cast<void *>(&timerTick));
  g_prof = nullptr;
}

void Profiler::timerTick()
{
  g_prof->sort();
  g_prof->frame();
  g_prof->cleanup();
}

void Profiler::addSlice(const char *name, const TimeUnit time, const bool isApi)
{
  if(isApi && !m_showApi)
    return;

  Timing &slice { m_data[name] };
  slice.timings[0] += time;
  ++slice.calls;
  if(slice.totalCount < slice.timings.size())
    ++slice.totalCount;
}

void Profiler::sort()
{
  std::sort(m_sorted.begin(), m_sorted.end(),
    [this](const DataStorePair *a, const DataStorePair *b) -> bool {
      switch(m_sortCol) {
      case 0:
        if(m_sortAsc)
          return (strcmp(a->first, b->first) < 0);
        else
          return (strcmp(a->first, b->first) > 0);
      case 1:
        if(m_sortAsc)
          return (a->second.calls < b->second.calls);
        else
          return (a->second.calls > b->second.calls);
      case 2:
        if(m_sortAsc)
          return (a->second.timings[0] < b->second.timings[0]);
        else
          return (a->second.timings[0] > b->second.timings[0]);
      case 3:
        if(m_sortAsc)
          return (a->second.average < b->second.average);
        else
          return (a->second.average > b->second.average);
      default:
        throw imgui_error { "unknown column" };
      }
    }
  );
}

void Profiler::cleanup()
{
  m_sorted.clear();

  auto it { m_data.begin() };
  while(it != m_data.end()) {
    auto target { it++ };
    if(!target->second.calls) {
      m_data.erase(target);
      continue;
    }

    auto &slice { target->second };
    slice.average = slice.timings[0];
    for(int i { std::min<int>(slice.totalCount, slice.timings.size() - 1) }; i >= 0; --i) {
      slice.average += slice.timings[i + 1];
      slice.timings[i + 1] = slice.timings[i];
    }
    slice.timings[0] = TimeUnit {};
    slice.calls = 0;
    slice.average /= slice.totalCount;

    m_sorted.push_back(&*target);
  }
}


void Profiler::frame()
{
  if(m_data.empty())
    return;

  if(!Resource::isValid(m_ctx))
    m_ctx = new Context { "Profiler", ReaImGuiConfigFlags_NoSavedSettings };

  m_ctx->keepAlive();
  m_ctx->enterFrame();
  ImGui::SetNextWindowSize({ 400, 300 }, ImGuiCond_Once);
  if(ImGui::Begin("Profiler"))
    window();
  ImGui::End();
}

static void showTime(Profiler::TimeUnit time)
{
  if(time.count() == 0)
    return;

  struct Unit { const char *si; Color col; };
  static Unit units[] {
    { "ns", 0xffffff80 }, { "us", 0xffffffff }, { "ms", 0xffff80ff },
    { "s",  0xff0000ff },
  };
  const size_t decade
    { std::min<size_t>(log10(time.count()) / 3, std::size(units) - 1) };
  time /= pow(1000, decade);
  ImGui::TextColored(units[decade].col, "%3lld %-2s",
    static_cast<long long int>(time.count()), units[decade].si);
}

void Profiler::window()
{
  const ImGuiStyle &style { ImGui::GetStyle() };
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, floor(style.FramePadding.y * 0.60f)));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, floor(style.ItemSpacing.y * 0.60f)));

  const ImGuiIO &io { ImGui::GetIO() };
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
  ImGui::Spacing();

  ImGui::Checkbox("API calls", &m_showApi);

  ImGui::PopStyleVar(2);
  ImGui::Spacing();

  constexpr ImGuiTableFlags tableFlags {
    ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit |
    ImGuiTableFlags_RowBg   | ImGuiTableFlags_Sortable       |
    ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX
  };
  if(!ImGui::BeginTable("slices", 4, tableFlags, { -FLT_MIN, -FLT_MIN }))
    return;
  ImGui::TableSetupScrollFreeze(0, 1);
  ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
  ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_PreferSortDescending);
  ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_PreferSortDescending);
  ImGui::TableSetupColumn("Average",
    ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortDescending);
  if(ImGuiTableSortSpecs *sort { ImGui::TableGetSortSpecs() }) {
    m_sortCol = sort->Specs->ColumnIndex;
    m_sortAsc = sort->Specs->SortDirection == ImGuiSortDirection_Ascending;
  }
  ImGui::TableHeadersRow();

  ImGuiListClipper clipper;
  clipper.Begin(m_sorted.size());
  while(clipper.Step()) {
    for(int i { clipper.DisplayStart }; i < clipper.DisplayEnd; ++i) {
      const auto &[name, slice] { *m_sorted[i] };
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(name);
      ImGui::TableNextColumn();
      ImGui::Text("%zu", slice.calls);
      ImGui::TableNextColumn();
      showTime(slice.timings[0]);
      ImGui::TableNextColumn();
      showTime(slice.average);
    }
  }

  ImGui::EndTable();
}

Profiler::Slice::Slice(const char *name, const bool isApi)
  : m_name { name }, m_isApi { isApi },
    m_start { std::chrono::steady_clock::now() }
{
}

Profiler::Slice::~Slice()
{
  const auto end { std::chrono::steady_clock::now() };
  const auto dur { std::chrono::duration_cast<TimeUnit>(end - m_start) };
  g_prof->addSlice(m_name, dur, m_isApi);
}
