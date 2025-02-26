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

#ifndef REAIMGUI_RENDERER_HPP
#define REAIMGUI_RENDERER_HPP

#include <array>
#include <memory>

class Renderer;
class RendererFactory;
class Window;
struct ImVec2;
struct ImVec4;

struct RendererType {
  enum Flags {
    Available        = 1<<0,
    CanForceSoftware = 1<<1,
  };

  struct Register {
    Register(RendererType *);
  };

  static const RendererType *bestMatch(const char *id);
  static const RendererType *head();

  char priority, flags;
  const char *id, *name, *displayName;
  std::unique_ptr<Renderer>(*creator)(RendererFactory *, Window *);
  RendererType *next;
};

class RendererFactory {
public:
  RendererFactory();

  const char *name() const { return m_type->name; }
  std::unique_ptr<Renderer> create(Window *);

  template<typename T>
  auto getSharedData() const { return std::static_pointer_cast<T>(m_shared.lock()); }

  template<typename T>
  void setSharedData(T d) { m_shared = d; }

  bool wantSoftware() const { return m_forceSoftware; }

private:
  const RendererType *m_type;
  std::weak_ptr<void> m_shared;
  bool m_forceSoftware;
};

class Renderer {
public:
  static void install();

  template<typename T>
  static std::unique_ptr<Renderer> create(RendererFactory *factory, Window *window)
  {
    return std::make_unique<T>(factory, window);
  }

  Renderer(Window *);
  virtual ~Renderer();

  virtual void setSize(ImVec2) = 0;
  virtual void render(void *) = 0;
  virtual void swapBuffers(void *) = 0;

protected:
  class ProjMtx {
  public:
    ProjMtx(const ImVec2 &pos, const ImVec2 &size, bool flip = false);
    const float *operator&() const { return &m_data[0][0]; }

  private:
    std::array<std::array<float, 4>, 4> m_data;
  };
  static_assert(sizeof(ProjMtx) == sizeof(float[4][4]));

  struct ClipRect {
    ClipRect(const ImVec4 &rect, const ImVec2 &offset, const ImVec2 &scale);
    operator bool() const;
    long left, top, right, bottom;
  };

  Window *m_window;
};

#define REGISTER_RENDERER(priority, id, name, creator, flags)   \
  static RendererType rendererType_##id                         \
    {priority, flags, #id, "reaper_imgui_" #id, name, creator}; \
  RendererType::Register regRenderer_##id {&rendererType_##id};

#endif
