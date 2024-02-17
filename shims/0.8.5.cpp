#include "shims.hpp"

SHIM("0.8.5",
  (void, PushTabStop, ImGui_Context*)
  (void, PopTabStop,  ImGui_Context*)
);

// dear imgui v1.89.4 breaking changes
SHIM_ALIAS(0_1, PushAllowKeyboardFocus, PushTabStop)
SHIM_ALIAS(0_1, PopAllowKeyboardFocus,  PopTabStop)
