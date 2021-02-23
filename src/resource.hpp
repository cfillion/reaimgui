#ifndef REAIMGUI_RESOURCE_HPP
#define REAIMGUI_RESOURCE_HPP

#include <memory>

class Resource {
public:
  Resource();
  virtual ~Resource();

  static bool exists(Resource *);

protected:
  virtual void heartbeat() = 0;

private:
  class Timer;
  std::shared_ptr<Timer> m_timer;
};

#endif
