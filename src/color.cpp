#include "color.hpp"

#include <imgui/imgui.h>

uint32_t Color::rgba2abgr(const uint32_t rgba)
{
  return
    (rgba >> 24 & 0x000000FF) | // red
    (rgba >> 8  & 0x0000FF00) | // green
    (rgba << 8  & 0x00FF0000) | // blue
    (rgba << 24 & 0xFF000000) ; // alpha
}

Color::Color(const uint32_t rgba, const bool alpha)
{
  uint32_t i {};
  auto &[r, g, b, a] { m_store };

  if(alpha)
    a = (rgba >> (8 * i++) & 0xFF) / 255.f;
  else
    a = 1.0f;
  b   = (rgba >> (8 * i++) & 0xFF) / 255.f;
  g   = (rgba >> (8 * i++) & 0xFF) / 255.f;
  r   = (rgba >> (8 * i++) & 0xFF) / 255.f;
}

Color::Color(const ImVec4 &vec, const bool alpha)
  : m_store { vec.x, vec.y, vec.z, alpha ? vec.w : 1.0f }
{
}

Color::Color(const float rgba[4], const bool alpha)
  : m_store { rgba[0], rgba[1], rgba[2], alpha ? rgba[3] : 1.0f }
{
}

Color::operator ImVec4() const
{
  auto [r, g, b, a] { m_store };
  return ImVec4{r, g, b, a};
}

uint32_t Color::pack(const bool alpha, const uint32_t extra) const
{
  uint32_t rgba {}, i {};
  auto [r, g, b, a] { m_store };

  if(alpha)
    rgba |= static_cast<uint32_t>(a * 0xFF) << (8 * i++);
  else if(extra) // preserve unused bits as-is (eg. REAPER's color enable flag)
    rgba |= extra & 0xFF000000;
  rgba   |= static_cast<uint32_t>(b * 0xFF) << (8 * i++);
  rgba   |= static_cast<uint32_t>(g * 0xFF) << (8 * i++);
  rgba   |= static_cast<uint32_t>(r * 0xFF) << (8 * i++);

  return rgba;
}

void Color::unpack(float rgba[4]) const
{
  std::tie(rgba[0], rgba[1], rgba[2], rgba[3]) = m_store;
}
