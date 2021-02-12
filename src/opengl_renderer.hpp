#ifndef REAIMGUI_OPENGL_RENDERER_HPP
#define REAIMGUI_OPENGL_RENDERER_HPP

#include <array>

class Color;
struct ImDrawData;

class OpenGLRenderer {
public:
  // minimum OpenGL version
  static constexpr int MIN_MAJOR { 3 };
  static constexpr int MIN_MINOR { 2 };

  OpenGLRenderer();
  ~OpenGLRenderer();

  void draw(ImDrawData *, const Color &clearColor);

private:
  void initShaders();
  void initFontTex();

  unsigned int m_vbo, m_program;
  std::array<unsigned int, 2> m_buffers;
  std::array<unsigned int, 1> m_textures; // make this a vector for image support?
  std::array<unsigned int, 5> m_locations;
};

#endif
