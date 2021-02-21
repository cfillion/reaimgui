#include "api_helper.hpp"

// Inputs Utilities: Mouse
// - To refer to a mouse button, you may use named enums in your code e.g. ImGuiMouseButton_Left, ImGuiMouseButton_Right.
// - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
// - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
// IMGUI_API bool          IsMouseDown(ImGuiMouseButton button);                               // is mouse button held?
// IMGUI_API bool          IsMouseClicked(ImGuiMouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down)
// IMGUI_API bool          IsMouseReleased(ImGuiMouseButton button);                           // did mouse button released? (went from Down to !Down)
DEFINE_API(bool, IsMouseDoubleClicked, ((ImGui_Context*,ctx))
((int,button)),
"Did mouse button double-clicked? (note that a double-click will also report IsMouseClicked() == true)",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::IsMouseDoubleClicked(button);
});
// IMGUI_API bool          IsMouseHoveringRect(const ImVec2& r_min, const ImVec2& r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
// IMGUI_API bool          IsMousePosValid(const ImVec2* mouse_pos = NULL);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
// IMGUI_API bool          IsAnyMouseDown();                                                   // is any mouse button held?
// IMGUI_API ImVec2        GetMousePos();                                                      // shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
// IMGUI_API ImVec2        GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
// IMGUI_API bool          IsMouseDragging(ImGuiMouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)

DEFINE_API(void, GetMouseDragDelta, ((ImGui_Context*,ctx))
((double*,API_W(x)))((double*,API_W(y)))
((int*,API_RO(button)))((double*,API_RO(lockThreshold))),
R"(Return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold).

Default values: button = ImGui_MouseButton_Left, lockThreshold = -1.0)",
{
  ENTER_CONTEXT(ctx);
  const ImVec2 &delta {
    ImGui::GetMouseDragDelta(valueOr(API_RO(button), ImGuiMouseButton_Left),
      valueOr(API_RO(lockThreshold), -1.0))
  };
  *API_W(x) = delta.x, *API_W(y) = delta.y;
});

DEFINE_API(void, ResetMouseDragDelta, ((ImGui_Context*,ctx))
((int*,API_RO(button))),
"Default values: button = ImGui_MouseButton_Left",
{
  ENTER_CONTEXT(ctx);
  ImGui::ResetMouseDragDelta(valueOr(API_RO(button), ImGuiMouseButton_Left));
});

// IMGUI_API ImGuiMouseCursor GetMouseCursor();                                                // get desired cursor type, reset in ImGui::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
// IMGUI_API void          SetMouseCursor(ImGuiMouseCursor cursor_type);                       // set desired cursor type
// IMGUI_API void          CaptureMouseFromApp(bool want_capture_mouse_value = true);          // attention: misleading name! manually override io.WantCaptureMouse flag next frame (said flag is entirely left for your application to handle). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse_value;" after the next NewFrame() call.
