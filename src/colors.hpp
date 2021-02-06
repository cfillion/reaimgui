#ifndef REAIMGUI_COLORS_HPP
#define REAIMGUI_COLORS_HPP

namespace Color {
  inline unsigned int pack(const float r, const float g, const float b, const float *a = nullptr)
  {
    unsigned int col {}, i {};
    if(a)
      col |= static_cast<unsigned int>(*a * 0xFF) << (8 * i++);
    col   |= static_cast<unsigned int>( b * 0xFF) << (8 * i++);
    col   |= static_cast<unsigned int>( g * 0xFF) << (8 * i++);
    col   |= static_cast<unsigned int>( r * 0xFF) << (8 * i++);
    return col;
  }

  inline void unpack(const unsigned int col, float &r, float &g, float &b, float *a = nullptr)
  {
    unsigned int i {};
    if(a)
      *a = (col >> (8 * i++) & 0xFF) / 255.f;
    b    = (col >> (8 * i++) & 0xFF) / 255.f;
    g    = (col >> (8 * i++) & 0xFF) / 255.f;
    r    = (col >> (8 * i++) & 0xFF) / 255.f;
  }
};

#endif
