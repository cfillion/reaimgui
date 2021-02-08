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

  inline void unpack(const unsigned int col, ImVec4 &out, const bool alpha = true)
  {
    unpack(col, out.x, out.y, out.z, alpha ? &out.w : nullptr);

    if(!alpha)
      out.w = 1.0f;
  }

  inline unsigned int rgba2abgr(const unsigned int rgba)
  {
    return
      (rgba >> 24 & 0x000000FF) | // red
      (rgba >> 8  & 0x0000FF00) | // green
      (rgba << 8  & 0x00FF0000) | // blue
      (rgba << 24 & 0xFF000000) ; // alpha
  }
};

#endif
