#ifndef REAIMGUI_API_HPP
#define REAIMGUI_API_HPP

#include "errors.hpp"

#include <string>

class API {
public:
  static void registerAll();
  static void unregisterAll();
  static void handleError(const char *fnName, const reascript_error &);
  static void handleError(const char *fnName, const imgui_error &);

  API(const char *name, void *cImpl, void *reascriptImpl, void *definition);
  ~API();

private:
  struct RegInfo {
    std::string key;
    void *value;
    void announce() const;
  } m_regs[3];
};

#endif
