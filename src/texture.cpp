/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

size_t TextureManager::touch(const Texture &tex)
{
  const auto [begin, end]
    { std::equal_range(m_textures.begin(), m_textures.end(), tex.user) };
  auto it { std::lower_bound(begin, end, tex.scale) };

  const auto now { static_cast<float>(ImGui::GetTime()) };

  if(it == end || it->user != tex.user || it->scale != tex.scale) {
    it = m_textures.insert(it, tex);
    ++m_version;
  }

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
    ++(it->version);

  ++m_version;
}

void TextureManager::cleanup()
{
  const float ttl { ImGui::GetIO().ConfigMemoryCompactTimer };
  const auto cutoff { static_cast<float>(ImGui::GetTime()) - ttl };
  const auto versionIfChange { m_version + 1 };

  auto it { m_textures.begin() };
  while(it != m_textures.end()) {
    if(!it->isValid()) {
      auto nextObjIt { it };
      do { ++nextObjIt; }
      while(nextObjIt != m_textures.end() && nextObjIt->user == it->user);

      it = m_textures.erase(it, nextObjIt);
      m_version = versionIfChange;
      continue;
    }

    if(it->lastTimeActive >= cutoff)
      ++it;
    else {
      it->compact();
      it = m_textures.erase(it);
      m_version = versionIfChange;
    }
  }
}

void TextureManager::update(TextureCookie *cookie, const CommandRunner &runner) const
{
  // There is no need for it now, but we might eventually want to have this
  // allow selecting only textures of a given scale (eg. if the GDK backend
  // ever gain multi-DPI capability.)

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

TextureCookie::TextureCookie()
  : m_version {}
{
}

void TextureCookie::doCommand(const TextureCmd &cmd)
{
  struct MakeCrumb {
    Crumb operator()(const Texture &tex)
    {
      return { tex.user, tex.scale, tex.version };
    };
  };

  auto crumb { m_crumbs.begin() + cmd.offset };

  switch(cmd.type) {
  case TextureCmd::Insert: {
    // assumes the manager stores textures contiguously!
    const Texture *tex { &cmd.manager->get(cmd.offset) };
    std::transform(tex, tex + cmd.size, std::inserter(m_crumbs, crumb),
                   MakeCrumb {});
    break;
  }
  case TextureCmd::Update: {
    auto texture { &cmd.manager->get(cmd.offset) };
    const auto end { crumb + cmd.size };
    while(crumb < end)
      (crumb++)->version = (texture++)->version;
    break;
  }
  case TextureCmd::Remove: {
    m_crumbs.erase(crumb, crumb + cmd.size);
    break;
  }}
}
