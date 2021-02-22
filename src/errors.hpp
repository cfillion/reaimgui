#ifndef REAIMGUI_ERRORS_HPP
#define REAIMGUI_ERRORS_HPP

#include <stdexcept>

#define DEFINE_EXCEPT(type)                \
  class type : public std::runtime_error { \
  public:                                  \
    using runtime_error::runtime_error;    \
  }

DEFINE_EXCEPT(reascript_error);
DEFINE_EXCEPT(imgui_error);

#undef DEFINE_EXCEPT

#endif
