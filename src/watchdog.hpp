#ifndef REAIMGUI_WATCHDOG_HPP
#define REAIMGUI_WATCHDOG_HPP

#include <memory>

class Watchdog {
public:
  static std::shared_ptr<Watchdog> get();

  Watchdog();
  ~Watchdog();
};

#endif
