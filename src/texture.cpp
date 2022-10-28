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

#include "texture.hpp"

#include <algorithm>
#include <imgui/imgui.cpp>
#include <iterator>

TextureManager::TextureManager()
  : m_version {}
{
}

size_t TextureManager::touch(void *object, const float scale, const PixelGetter getPixels)
{
  const auto [begin, end]
    { std::equal_range(m_textures.begin(), m_textures.end(), object) };
  auto it { std::lower_bound(begin, end, scale) };

  const auto now { static_cast<float>(ImGui::GetTime()) };

  if(it == end) {
    it = m_textures.insert(it, { object, scale, 0, now, getPixels });
    ++m_version;
  }
  else
    it->lastTimeActive = now;

  return std::distance(m_textures.begin(), it);
}

void TextureManager::remove(void *object)
{
  const auto [begin, end]
    { std::equal_range(m_textures.begin(), m_textures.end(), object) };

  if(begin != end) {
    m_textures.erase(begin, end);
    ++m_version;
  }
}

void TextureManager::invalidate(void *object)
{
  const auto [begin, end]
    { equal_range(m_textures.begin(), m_textures.end(), object) };

  for(auto it { begin }; it < end; ++it)
    ++it->version;

  ++m_version;
}

void TextureManager::update(TextureCookie *cookie, const CommandRunner &runner) const
{
  constexpr auto NullCmd { static_cast<TextureCmd::Type>(-1) };

  if(m_version == cookie->m_version)
    return;

  cookie->m_version = m_version;
  cookie->m_crumbs.reserve(m_textures.size());

  TextureCmd cmd { this, NullCmd };

  for(size_t i {}, j {}; i < m_textures.size() &&
                         j < cookie->m_crumbs.size(); ++i, ++j) {
    const Texture &tex { m_textures[i] };
    const auto &crumb { cookie->m_crumbs[j] };

    TextureCmd::Type wantCmd;

    if(tex.user < crumb.user ||
        (tex.user == crumb.user && tex.scale < crumb.scale)) {
      --j;
      wantCmd = TextureCmd::Insert;
    }
    else if(tex.user > crumb.user || tex.scale > crumb.scale) {
      --i;
      wantCmd = TextureCmd::Remove;
    }
    else if(crumb.version != tex.version)
      wantCmd = TextureCmd::Update;
    else
      wantCmd = NullCmd;

    if(cmd.type == wantCmd) {
      // collect more into a previously prepared command
      ++cmd.size;
      continue;
    }
    else if(cmd.type != NullCmd) {
      // execute the previous completed command
      runner(cmd);
      cookie->doCommand(cmd);
      j = i;
    }

    // prepare the next command
    if(cmd.type != TextureCmd::Remove)
      cmd.offset += cmd.size;
    cmd.size = 1;
    cmd.type = wantCmd;
  }

  // the loop may end before sending its last command
  if(cmd.type != NullCmd) {
    runner(cmd);
    cookie->doCommand(cmd);
  }

  if(const auto diff { static_cast<long long>(cookie->m_crumbs.size()) -
                       static_cast<long long>(m_textures.size()) }) {
    cmd.type = diff < 0 ? TextureCmd::Insert : TextureCmd::Remove;
    cmd.offset += cmd.size;
    cmd.size = std::abs(diff);

    runner(cmd);
    cookie->doCommand(cmd);
  }
}

unsigned char *TextureManager::getPixels(const size_t index, int *width, int *height) const
{
  const Texture &tex { m_textures[index] };
  return tex.getPixels(tex.user, tex.scale, width, height);
}

TextureCookie::TextureCookie()
  : m_version {}
{
}

void TextureCookie::doCommand(const TextureCmd &cmd)
{
  struct MakeCrumb {
    Crumb operator()(const TextureManager::Texture &tex)
    {
      return { tex.user, tex.scale, tex.version };
    };
  };

  const auto &textures { cmd.manager->m_textures };

  switch(cmd.type) {
  case TextureCmd::Insert: {
    const auto it { textures.begin() + cmd.offset };
    std::transform(it, it + cmd.size,
                   std::inserter(m_crumbs, m_crumbs.begin() + cmd.offset),
                   MakeCrumb {});
    break;
  }
  case TextureCmd::Update: {
    auto crumb   { m_crumbs.begin() + cmd.offset };
    auto texture { textures.begin() + cmd.offset };
    const auto end { crumb + cmd.size };
    while(crumb < end)
      (crumb++)->version = (texture++)->version;
    break;
  }
  case TextureCmd::Remove: {
    const auto it { m_crumbs.begin() + cmd.offset };
    m_crumbs.erase(it, it + cmd.size);
    break;
  }}
}
