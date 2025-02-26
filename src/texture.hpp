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

#ifndef REAIMGUI_TEXTURE_HPP
#define REAIMGUI_TEXTURE_HPP

#include <functional>
#include <vector>

class TextureCookie;
class TextureManager;
struct TextureCmd;

using TextureVersion = unsigned int;

class Texture {
public:
  using GetPixelsFunc = const unsigned char *(*)(const Texture &,
                                                 int *width, int *height);
  using CompactFunc   = bool(*)(const Texture &);
  using IsValidFunc   = bool(*)(void *object);

  Texture(void *user, float scale, GetPixelsFunc getPixels,
    IsValidFunc isValid = nullptr, CompactFunc compact = nullptr)
    : m_user {user}, m_scale {scale}, m_getPixels {getPixels},
      m_compact {compact}, m_isValid {isValid},
      m_version {0u}, m_lastTimeActive {0.f}
  {}

  void *object() const { return m_user; }
  float scale()  const { return m_scale; }

  bool isSame(void *user, const float scale) const
  {
    return m_user == user && m_scale == scale;
  }

  const unsigned char *getPixels(int *width, int *height) const
  {
    return m_getPixels(*this, width, height);
  }

  bool isValid() const
  {
    return m_isValid ? m_isValid(m_user) : true;
  }

  bool compact() const
  {
    return m_compact ? m_compact(*this) : true;
  }

private:
  friend TextureManager;
  friend TextureCookie;

  void *m_user;
  float m_scale;
  GetPixelsFunc m_getPixels;
  CompactFunc   m_compact;
  IsValidFunc   m_isValid;
  TextureVersion m_version;
  float m_lastTimeActive;
};

class TextureManager {
public:
  using CommandRunner = std::function<void (const TextureCmd &)>;

  TextureManager();

  size_t touch(Texture &&);
  template<typename... Args>
  size_t touch(Args &&...args) { return touch(Texture {args...}); }
  const Texture &get(size_t i) const { return m_textures[i]; }
  void invalidate(void *object);

  // invalidates all indices given by touch()
  void cleanup();
  void remove(void *object);

  void update(TextureCookie *, const CommandRunner &) const;

private:
  std::vector<Texture> m_textures;
  std::vector<size_t> m_sorted;
  TextureVersion m_version;
};

class TextureCookie {
public:
  TextureCookie();

private:
  friend TextureManager;

  struct Crumb {
    void *user;
    float scale;
    TextureVersion version;
  };

  void doCommand(const TextureCmd &);

  TextureVersion m_version;
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
