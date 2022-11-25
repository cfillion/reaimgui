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
struct TextureCmd;

class TextureManager {
public:
  using GetPixelsFunc = const unsigned char *(*)(void *object, float scale,
                                               int *width, int *height);
  using IsValidFunc   = bool(*)(void *object);
  using CommandRunner = std::function<void (const TextureCmd &)>;

  TextureManager();

  size_t touch(void *object, float scale, GetPixelsFunc, IsValidFunc = nullptr);
  void remove(void *object);
  void invalidate(void *object);

  void cleanup();
  void update(TextureCookie *, const CommandRunner &) const;
  const unsigned char *getPixels(size_t index, int *width, int *height) const;

private:
  friend TextureCookie;

  struct Texture {
    Texture(void *user, float scale, GetPixelsFunc getPixels,
            IsValidFunc isValid, float now)
      : user { user }, scale { scale }, getPixels { getPixels },
        isValid { isValid }, version { 0u }, lastTimeActive { now }
    {}

    void *user;
    float scale;
    GetPixelsFunc getPixels;
    IsValidFunc   isValid;

    unsigned int version;
    float lastTimeActive;

    operator void *() const { return user; }
    operator float() const { return scale; }
  };

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
};

#endif
