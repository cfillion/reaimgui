#ifndef REAIMGUI_BACKEND_HPP
#define REAIMGUI_BACKEND_HPP

#include <memory>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class Context;
struct ImDrawData;

class Backend {
public:
  static std::unique_ptr<Backend> create(Context *);

  virtual ~Backend() {}

  virtual void beginFrame() {}
  virtual void drawFrame(ImDrawData *) = 0;
  virtual void endFrame() {}
  virtual float deltaTime() = 0;
  virtual float scaleFactor() const = 0;
  virtual bool handleMessage(unsigned int msg, WPARAM, LPARAM) = 0;
};

#endif
