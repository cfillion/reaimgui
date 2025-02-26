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

#include "texture.hpp"

#include <algorithm>
#include <imgui/imgui.h>
#include <iterator>
#include <numeric>

class Comparator {
public:
  Comparator(const TextureManager *manager) : m_manager {manager} {}

  bool operator()(const size_t ida, const size_t idb) const
  {
    const Texture &a {m_manager->get(ida)}, &b {m_manager->get(idb)};
    if(a.object() == b.object())
      return a.scale() < b.scale();
    return a.object() < b.object();
  }

  bool operator()(const size_t index, void *user) const
  {
    return m_manager->get(index).object() < user;
  }

  bool operator()(void *user, const size_t index) const
  {
    return user < m_manager->get(index).object();
  }

  bool operator()(const size_t index, const float scale) const
  {
    return m_manager->get(index).scale() < scale;
  }

  bool operator()(const float scale, const size_t index) const
  {
    return scale < m_manager->get(index).scale();
  }

private:
  const TextureManager *m_manager;
};

TextureManager::TextureManager()
  : m_version {}
{
}

size_t TextureManager::touch(Texture &&tex)
{
  const Comparator comparator {this};
  const auto [begin, end]
    {std::equal_range(m_sorted.begin(), m_sorted.end(), tex.m_user, comparator)};
  auto it {std::lower_bound(begin, end, tex.m_scale, comparator)};

  if(it == end || !m_textures[*it].isSame(tex.m_user, tex.m_scale)) {
    tex.m_version = m_version;
    const Texture &inserted {m_textures.emplace_back(std::move(tex))};
    it = m_sorted.emplace(it, &inserted - &m_textures.front());
    ++m_version;
  }

  m_textures[*it].m_lastTimeActive = ImGui::GetTime();

  return *it;
}

void TextureManager::invalidate(void *object)
{
  const Comparator comparator {this};
  const auto [begin, end]
    {equal_range(m_sorted.begin(), m_sorted.end(), object, comparator)};

  for(auto it {begin}; it < end; ++it)
    ++(m_textures[*it].m_version);

  ++m_version;
}

void TextureManager::remove(void *object)
{
  const Comparator comparator {this};
  const auto [begin, end]
    {std::equal_range(m_sorted.begin(), m_sorted.end(), object, comparator)};

  if(begin == end)
    return;

  std::sort(begin, end);
  const size_t first {*begin}, last {*std::prev(end)};

  size_t i {};
  m_textures.erase(std::remove_if(m_textures.begin(), m_textures.end(),
  [&](const Texture &) {
    const bool rm {i >= first && i <= last};
    ++i;
    return rm;
  }), m_textures.end());

  const auto rangeSize {end - begin};
  for(size_t &i : m_sorted) {
    if(i >= *begin)
      i -= rangeSize;
  }

  m_sorted.erase(begin, end);
  ++m_version;
}

void TextureManager::cleanup()
{
  const float ttl {ImGui::GetIO().ConfigMemoryCompactTimer};
  const auto cutoff {static_cast<float>(ImGui::GetTime()) - ttl};
  const auto isExpired {[cutoff](const Texture &tex) {
    return !tex.isValid() || (tex.m_lastTimeActive <= cutoff && tex.compact());
  }};
  const auto newEnd
    {std::remove_if(m_textures.begin(), m_textures.end(), isExpired)};

  if(newEnd == m_textures.end())
    return;

  ++m_version;
  m_textures.erase(newEnd, m_textures.end());
  m_sorted.resize(m_textures.size());
  std::iota(m_sorted.begin(), m_sorted.end(), 0);
  std::sort(m_sorted.begin(), m_sorted.end(), Comparator {this});
}

void TextureManager::update(TextureCookie *cookie, const CommandRunner &runner) const
{
  // Every window's renderer must call this function every frame to stay in sync.
  // Failure to do so may cause an insertion request for a resource that has
  // been deleted between cleanup() at the beginning of the frame and
  // rendering at the end of the frame.

  // There is no need for it now, but we might eventually want to have this
  // allow selecting only textures of a given scale (eg. if the GDK backend
  // ever gain multi-DPI capability.)

  const auto NullCmd {static_cast<TextureCmd::Type>(-1)};

  if(m_version == cookie->m_version)
    return;

  cookie->m_version = m_version;
  cookie->m_crumbs.reserve(m_textures.size());

  TextureCmd cmd {this, NullCmd};

  for(size_t i {}, j {}; i < m_textures.size() &&
                         j < cookie->m_crumbs.size(); ++i, ++j) {
    const Texture &tex {m_textures[i]};
    const auto &crumb {cookie->m_crumbs[j]};

    TextureCmd::Type wantCmd;

    if(!tex.isSame(crumb.user, crumb.scale)) {
      --i;
      wantCmd = TextureCmd::Remove;
    }
    else if(crumb.version != tex.m_version)
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
      j = i + (wantCmd == TextureCmd::Remove);
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

  if(const auto diff {static_cast<long long>(cookie->m_crumbs.size()) -
                      static_cast<long long>(m_textures.size())}) {
    if(diff < 0) {
      cmd.type = TextureCmd::Insert;
      cmd.offset = cookie->m_crumbs.size();
    }
    else {
      cmd.type = TextureCmd::Remove;
      cmd.offset = m_textures.size();
    }
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
  auto crumb {m_crumbs.begin() + cmd.offset};

  switch(cmd.type) {
  case TextureCmd::Insert: {
    // assumes the manager stores textures contiguously!
    const Texture *tex {&cmd.manager->get(cmd.offset)};
    std::transform(tex, tex + cmd.size, std::inserter(m_crumbs, crumb),
      [](const Texture &tex) {
        return Crumb {tex.m_user, tex.m_scale, tex.m_version};
      });
    break;
  }
  case TextureCmd::Update: {
    auto texture {&cmd.manager->get(cmd.offset)};
    const auto end {crumb + cmd.size};
    while(crumb < end)
      (crumb++)->version = (texture++)->m_version;
    break;
  }
  case TextureCmd::Remove:
    m_crumbs.erase(crumb, crumb + cmd.size);
    break;
  }
}
