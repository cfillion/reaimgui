#ifndef REAIMGUI_API_HPP
#define REAIMGUI_API_HPP

#include <string>

class API {
public:
  static void registerAll();
  static void unregisterAll();

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
