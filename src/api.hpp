/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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

#ifndef REAIMGUI_API_HPP
#define REAIMGUI_API_HPP

#include "error.hpp"
#include "vernum.hpp"

#define API_PREFIX ImGui_

namespace API {
  void setup();
  void teardown();
  void handleError(const char *fnName, const reascript_error &);
  void handleError(const char *fnName, const imgui_error &);

  using LineNumber = unsigned short;
  struct StoreLineNumber { StoreLineNumber(LineNumber val); };

  struct Section {
    Section(const Section *parent, const char *file,
      const char *title, const char *help = nullptr);
    const Section *parent;
    const char *file, *title, *help;
  };

  struct PluginRegister {
    const char *key; void *value;
    void announce(bool) const;
  };

  class Callable { // all instances must be mutable (not in .rodata)!
  public:
    static const Callable *lookup(VerNum, const char *name);

    Callable(VerNum since, VerNum until, const char *name);
    VerNum version() const { return m_since; }
    const Callable *rollback(VerNum) const;

    virtual void *safeImpl()   const = 0;
    virtual void *unsafeImpl() const = 0;

  private:
    VerNum m_since, m_until;
    Callable *m_precursor;
  };

  class Symbol {
  public:
    enum Flags {
      TargetNative  = 1<<0,
      TargetScript  = 1<<1,
      TargetEELFunc = 1<<2,

      Variable = 1<<11,
    };

    Symbol();
    Symbol(const Symbol &) = delete;

    const Section *m_section;
    const Symbol  *m_next;
    LineNumber     m_line;

    virtual void announce(bool)      const = 0;
    virtual const char *name()       const = 0;
    virtual const char *definition() const = 0;
    virtual unsigned int flags()     const = 0;
    virtual VerNum version()         const = 0;
  };

  class ReaScriptFunc final : public Callable, Symbol {
  public:
    ReaScriptFunc(VerNum availableSince, void *impl,
                  const PluginRegister &native,
                  const PluginRegister &reascript,
                  const PluginRegister &desc);
    void announce(bool) const override;

    void *safeImpl()   const override { return m_regs[1].value; }
    void *unsafeImpl() const override { return m_impl; }

    const char *name() const override {
      return &m_regs[0].key[5]; /* strlen("-API_") */ }
    const char *definition() const override {
      return static_cast<const char *>(m_regs[2].value); }
    unsigned int flags() const override { return TargetNative | TargetScript; }
    VerNum version() const override { return Callable::version(); }

  private:
    void *m_impl;
    PluginRegister m_regs[3]; // native, reascript, definition
  };

  class ShimFunc final : public Callable {
  public:
    ShimFunc(VerNum since, VerNum until,
      const char *name, void *impl, void *unsafeImpl);

    void *safeImpl()   const override { return m_impl; }
    void *unsafeImpl() const override { return m_unsafeImpl; }

  private:
    void *m_impl, *m_unsafeImpl;
  };

  // All fields from this+size are treated as const char* of Callable names
  // resolve() replaces them with the addresses of their unsafe implementations
  class ImportTable {
  protected:
    ImportTable(VerNum, size_t);

  public:
    void resolve();
    ImportTable *m_next;

  private:
    void **offset(size_t);
    void **m_ftable;
    VerNum m_version;
  };
}

#ifdef _WIN32
#  define GENBINDINGS_API __declspec(dllexport)
#else
#  define GENBINDINGS_API __attribute__((visibility("default")))
#endif

extern "C" GENBINDINGS_API const API::Symbol *API_head();

#endif
