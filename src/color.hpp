#ifndef REAIMGUI_COLORS_HPP
#define REAIMGUI_COLORS_HPP

#include <cstdint>
#include <tuple>

struct ImVec4;

class Color {
public:
  static uint32_t rgba2abgr(const uint32_t rgba);
  static Color fromTheme(const uint32_t themeColor);

  Color(uint32_t rgba, bool alpha = true);
  Color(const ImVec4 &, bool alpha = true);
  Color(const float rgba[4], bool alpha = true);

  operator ImVec4() const;
  void unpack(float rgba[4]) const;
  uint32_t pack(bool alpha = true, uint32_t prewextra = 0) const;

  template <class F>
  constexpr decltype(auto) apply(F&& f) const
  {
    return std::apply(f, m_store);
  }

private:
  using Tuple = std::tuple<float, float, float, float>;
  Tuple m_store;
};

#endif
