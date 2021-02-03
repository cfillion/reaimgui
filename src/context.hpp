#ifndef REAIMGUI_CONTEXT_HPP
#define REAIMGUI_CONTEXT_HPP

#include <memory>

class Context {
public:
  static std::shared_ptr<Context> get();

  Context();
  virtual ~Context();

private:
  static std::shared_ptr<Context> create();
  static void timerTick();
};

#endif
