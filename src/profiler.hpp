#ifndef REAIMGUI_PROFILER_HPP
#define REAIMGUI_PROFILER_HPP

#include <array>
#include <chrono>
#include <unordered_map>
#include <vector>

class Context;

class Profiler {
public:
  using TimeUnit = std::chrono::nanoseconds;

  class Slice {
  public:
    Slice(const char *name, const bool isApi = false);
    ~Slice();

  private:
    const char *m_name;
    bool m_isApi;
    std::chrono::time_point<std::chrono::steady_clock> m_start;
  };

  Profiler();
  ~Profiler();

  void addSlice(const char *name, TimeUnit, bool isApi = false);

private:
  struct Timing {
    size_t calls { 0 }, totalCount { 0 };
    std::array<TimeUnit, 120> timings {};
    TimeUnit average;
  };

  static void timerTick();

  void sort();
  void cleanup();
  void frame();
  void window();

  using DataStore = std::unordered_map<const char *, Timing>;
  using DataStorePair = DataStore::value_type;

  Context *m_ctx;
  DataStore m_data;
  std::vector<const DataStorePair *> m_sorted;
  int m_sortCol;
  bool m_sortAsc, m_showApi;
};

extern Profiler *g_prof;

#endif
