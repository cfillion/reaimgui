#include "api_helper.hpp"

DEFINE_API(bool, TreeNode, (ImGui_Context*,ctx)
(const char*, label)(int*,API_RO(flags)),
"TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.",
{
  Context::check(ctx)->enterFrame();
  return ImGui::TreeNodeEx(label, valueOr(API_RO(flags), 0));
});

DEFINE_API(bool, TreeNodeEx, (ImGui_Context*,ctx)
(const char*, str_id)(const char*, label)(int*,API_RO(flags)),
"Helper variation to easily decorelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet(). See ImGui_TreeNode.",
{
  Context::check(ctx)->enterFrame();
  return ImGui::TreeNodeEx(str_id, valueOr(API_RO(flags), 0), "%s", label);
});

// IMGUI_API void          TreePush(const char* str_id);                                       // ~ Indent()+PushId(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
// IMGUI_API void          TreePush(const void* ptr_id = NULL);                                // "

DEFINE_API(void, TreePop, (ImGui_Context*,ctx),
"Unindent()+PopId()",
{
  Context::check(ctx)->enterFrame();
  ImGui::TreePop();
});

DEFINE_API(double, GetTreeNodeToLabelSpacing, (ImGui_Context*,ctx),
"Horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode",
{
  Context::check(ctx)->enterFrame();
  return ImGui::GetTreeNodeToLabelSpacing();
});

DEFINE_API(bool, CollapsingHeader, (ImGui_Context*,ctx)
(const char*, label)(bool*,API_RWO(visible))(int*,API_RO(flags)),
R"(If returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().

When 'visible' is provided: if 'true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if 'false' don't display the header.)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::CollapsingHeader(label, API_RWO(visible), valueOr(API_RO(flags), 0));
});

DEFINE_API(void, SetNextItemOpen, (ImGui_Context*,ctx)
(bool,isOpen)(int*,API_RO(cond)),
R"(Set next TreeNode/CollapsingHeader open state. Can also be done with the ImGui_TreeNodeFlags_DefaultOpen flag.

'cond' is ImGui_Cond_Always by default.)",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetNextItemOpen(isOpen, valueOr(API_RO(cond), ImGuiCond_Always));
});
