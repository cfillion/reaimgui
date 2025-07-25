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

#ifndef REAIMGUI_RESOURCE_HPP
#define REAIMGUI_RESOURCE_HPP

#include <cassert>

#include "../api/types.hpp"
#include "flat_set.hpp"

class Context;

class SubresourceData {
public:
  template<typename T>
  SubresourceData(T *data, void (*uninstaller)(Context *, T *))
    : m_ptr {data},
      m_deleter {reinterpret_cast<void(*)(Context *, void *)>(uninstaller)}
  {}
  SubresourceData(const SubresourceData &) = delete;
  SubresourceData(SubresourceData &&o) noexcept { *this = std::move(o); }
  SubresourceData &operator=(SubresourceData &&o) noexcept {
    m_ptr = o.m_ptr, m_deleter = o.m_deleter;
    o.m_ptr = nullptr;
    return *this;
  }
  ~SubresourceData() { assert(!m_ptr); };

  operator void *() const { return m_ptr; }
  void uninstall(Context *ctx) { m_deleter(ctx, m_ptr); m_ptr = nullptr; }

private:
  void *m_ptr;
  void (*m_deleter)(Context *, void *);
};

class Resource {
public:
  Resource();
  Resource(const Resource &) = delete;
  virtual ~Resource();

  void keepAlive();
  unsigned int uniqId() const { return m_uniqId; }

  virtual bool attachable(const Context *) const = 0;

  // context subresources
  virtual SubresourceData install(Context *) { throw nullptr; }
  virtual void update(Context *, void *) {}

  template<typename T>
  static bool isValid(T *userData)
  {
    if constexpr(std::is_same_v<Resource, std::remove_const_t<T>>)
      return g_rsx.contains(const_cast<Resource *>(userData)) && userData->isValid();

    auto resource {static_cast<const Resource *>(userData)};
    return isValid(resource) && resource->isInstanceOf<T>();
  }

  template<typename T, typename Fn>
  static void foreach(const Fn &&callback) // O(n) of all types, not per type
  {
    for(Resource *rs : g_rsx) {
      if(rs->isInstanceOf<T>())
        callback(static_cast<T *>(rs));
    }
  }

  static void destroyAll();
  static void bypassGCCheckOnce();
  static void testHeartbeat();

  template<typename T>
  bool isInstanceOf() const
  {
    // short-circuiting dynamic_cast for faster exact type match
    return typeid(*this) == typeid(T) || dynamic_cast<const T *>(this);
  }

protected:
  virtual bool heartbeat();
  virtual bool isValid() const;

private:
  struct Timer;

  static FlatSet<Resource *> g_rsx;
  static Timer *g_timer;

  unsigned int m_uniqId;
  signed char m_keepAlive;
  unsigned char m_flags;
};

API_REGISTER_OBJECT_TYPE(Resource);

#endif
