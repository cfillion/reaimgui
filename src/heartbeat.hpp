#ifndef REAIMGUI_HEARTBEAT_HPP
#define REAIMGUI_HEARTBEAT_HPP

#include <memory>

class Heartbeat {
public:
  static std::shared_ptr<Heartbeat> get();

  Heartbeat();
  ~Heartbeat();

private:
  static void tick();
};

#endif
