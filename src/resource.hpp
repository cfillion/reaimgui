#ifndef REAIMGUI_RESOURCE_HPP
#define REAIMGUI_RESOURCE_HPP

#include <memory>

class Resource {
public:
  Resource();
  Resource(const Resource &) = delete;
  virtual ~Resource();

  template<typename T>
  static bool exists(T *userData)
  {
    static_assert(!std::is_same_v<Resource, T>);

    // static_cast needed for dynamic_cast to check whether it's really a T
    Resource *resource { static_cast<Resource *>(userData) };
    return exists(resource) && dynamic_cast<T *>(resource);
  }

protected:
  virtual void heartbeat() = 0;

private:
  class Timer;
  std::shared_ptr<Timer> m_timer;
};

template<>
bool Resource::exists<Resource>(Resource *);

#endif
