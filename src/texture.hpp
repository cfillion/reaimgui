/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#ifndef REAIMGUI_TEXTURE_HPP
#define REAIMGUI_TEXTURE_HPP

#include <functional>
#include <vector>

class TextureCookie;
class TextureManager;
struct TextureCmd;

class Texture {
public:
  using GetPixelsFunc = const unsigned char *(*)(void *object, float scale,
                                                 int *width, int *height);
  using IsValidFunc   = bool(*)(void *object);

  Texture(void *user, float scale, GetPixelsFunc getPixels)
    : user { user }, scale { scale }, m_getPixels { getPixels },
      m_isValid { nullptr }, version { 0u }, lastTimeActive { 0.f }
  {}

  void *user;
  float scale;
  GetPixelsFunc m_getPixels;
  IsValidFunc   m_isValid;

  const unsigned char *getPixels(int *width, int *height) const
  {
    return m_getPixels(user, scale, width, height);
  }

  bool isValid() const
  {
    return m_isValid ? m_isValid(user) : true;
  }

  operator void*() const { return user;  }
  operator float() const { return scale; }

private:
  friend TextureManager;
  friend TextureCookie;

  unsigned int version;
  float lastTimeActive;
};

class TextureManager {
public:
  using CommandRunner = std::function<void (const TextureCmd &)>;

  TextureManager();

  size_t touch(const Texture &);
  template<typename... Args>
  size_t touch(Args &&...args) { return touch({ args... }); }
  const Texture &get(size_t i) const { return m_textures[i]; }
  void remove(void *object);
  void invalidate(void *object);

  void cleanup();
  void update(TextureCookie *, const CommandRunner &) const;

private:
  std::vector<Texture> m_textures;
  unsigned int m_version;
};

class TextureCookie {
public:
  TextureCookie();

private:
  friend TextureManager;

  struct Crumb {
    void *user;
    float scale;
    unsigned int version;
  };

  void doCommand(const TextureCmd &);

  unsigned int m_version;
  std::vector<Crumb> m_crumbs;
};

struct TextureCmd {
  const TextureManager *manager;
  enum Type { Insert, Update, Remove };
  Type type;
  size_t offset, size;

  const Texture &operator[](const size_t i) const
  {
    return manager->get(offset + i);
  }
};

#endif
