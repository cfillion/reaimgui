#include "shims.hpp"

struct ImGui_ListClipper;

SHIM("0.8.7",
  (void, ListClipper_IncludeRangeByIndices, ImGui_ListClipper*)
);

// dear imgui v1.89.6 breaking change
SHIM_ALIAS(0_5_10, ListClipper_ForceDisplayRangeByIndices,
  ListClipper_IncludeRangeByIndices)
