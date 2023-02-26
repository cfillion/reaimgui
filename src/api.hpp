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

#ifndef REAIMGUI_API_HPP
#define REAIMGUI_API_HPP

#include "error.hpp"

#ifdef _WIN32
#  ifdef IMPORT_GENBINDINGS_API
#    define GENBINDINGS_API __declspec(dllimport)
#  else
#    define GENBINDINGS_API __declspec(dllexport)
#  endif
#else
#  define GENBINDINGS_API __attribute__((visibility("default")))
#endif

namespace API {
  void announceAll(bool add);
  void handleError(const char *fnName, const reascript_error &);
  void handleError(const char *fnName, const imgui_error &);

  // internal helpers for genbindings
  class Symbol;
  GENBINDINGS_API const Symbol *head();

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

  class Symbol {
  public:
    enum Flags {
      TargetNative = 1<<0,
      TargetLua    = 1<<1,
      TargetEEL    = 1<<2,
      TargetEELOld = 1<<3,
      TargetPython = 1<<4,

      Variable = 1<<10,
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
  };

  class ReaScriptFunc : public Symbol {
  public:
    ReaScriptFunc(const PluginRegister &native,
                  const PluginRegister &reascript,
                  const PluginRegister &desc);
    void announce(bool) const override;

    const char *name() const override {
      return &m_regs[0].key[5]; /* strlen("-API_") */ }
    const char *definition() const override {
      return static_cast<const char *>(m_regs[2].value); }
    unsigned int flags() const override {
      return TargetNative | TargetLua | TargetEEL | TargetEELOld | TargetPython;
    }

  private:
    PluginRegister m_regs[3]; // native, reascript, definition
  };
}

#endif
