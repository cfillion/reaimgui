/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "helper.hpp"

API_SECTION("Tree Node");

API_FUNC(0_1, bool, TreeNode, (Context*,ctx)
(const char*,label) (RO<int*>,flags,ImGuiTreeNodeFlags_None),
R"(TreeNode functions return true when the node is open, in which case you need
to also call TreePop when you are finished displaying the tree node contents.)")
{
  FRAME_GUARD;
  return ImGui::TreeNodeEx(label, API_GET(flags));
}

API_FUNC(0_1, bool, TreeNodeEx, (Context*,ctx)
(const char*,str_id) (const char*,label) (RO<int*>,flags,ImGuiTreeNodeFlags_None),
R"(Helper variation to easily decorelate the id from the displayed string.
Read the [FAQ](https://dearimgui.com/faq) about why and how to use ID.
To align arbitrary text at the same level as a TreeNode you can use Bullet.)")
{
  FRAME_GUARD;
  return ImGui::TreeNodeEx(str_id, API_GET(flags), "%s", label);
}

API_FUNC(0_1, void, TreePush, (Context*,ctx)
(const char*,str_id),
R"(Indent()+PushID(). Already called by TreeNode when returning true,
but you can call TreePush/TreePop yourself if desired.)")
{
  FRAME_GUARD;
  ImGui::TreePush(str_id);
}

API_FUNC(0_1, void, TreePop, (Context*,ctx),
"Unindent()+PopID()")
{
  FRAME_GUARD;
  ImGui::TreePop();
}

API_FUNC(0_1, double, GetTreeNodeToLabelSpacing, (Context*,ctx),
R"(Horizontal distance preceding label when using TreeNode*() or Bullet()
== (GetFontSize + StyleVar_FramePadding.x*2) for a regular unframed TreeNode.)")
{
  FRAME_GUARD;
  return ImGui::GetTreeNodeToLabelSpacing();
}

API_FUNC(0_1, bool, CollapsingHeader, (Context*,ctx)
(const char*,label) (RWO<bool*>,p_visible) (RO<int*>,flags,ImGuiTreeNodeFlags_None),
R"(Returns true when opened but do not indent nor push into the ID stack
(because of the TreeNodeFlags_NoTreePushOnOpen flag).

This is basically the same as calling TreeNode(label, TreeNodeFlags_CollapsingHeader).
You can remove the _NoTreePushOnOpen flag if you want behavior closer to normal
TreeNode.

When 'visible' is provided: if 'true' display an additional small close button
on upper right of the header which will set the bool to false when clicked,
if 'false' don't display the header.)")
{
  FRAME_GUARD;
  // p_visible behavior differs from ImGui: false as input is treated the same
  // as NULL. This is because EEL doesn't have a NULL (0 = false), W<> never
  // receives a NULL, and RWO<>s aren't listed in the output values list.
  return ImGui::CollapsingHeader(label, openPtrBehavior(p_visible), API_GET(flags));
}

API_FUNC(0_1, void, SetNextItemOpen, (Context*,ctx)
(bool,is_open) (RO<int*>,cond,ImGuiCond_Always),
R"(Set next TreeNode/CollapsingHeader open state.
Can also be done with the TreeNodeFlags_DefaultOpen flag.)")
{
  FRAME_GUARD;
  ImGui::SetNextItemOpen(is_open, API_GET(cond));
}

API_FUNC(0_1, bool, IsItemToggledOpen, (Context*,ctx),
"Was the last item open state toggled? Set by TreeNode.")
{
  FRAME_GUARD;
  return ImGui::IsItemToggledOpen();
}

API_SUBSECTION("Flags", "For TreeNode, TreeNodeEx and CollapsingHeader.");

API_ENUM(0_1, ImGui, TreeNodeFlags_None,     "");
API_ENUM(0_1, ImGui, TreeNodeFlags_Selected, "Draw as selected.");
API_ENUM(0_1, ImGui, TreeNodeFlags_Framed,
  "Draw frame with background (e.g. for CollapsingHeader).");
API_ENUM(0_9, ImGui, TreeNodeFlags_AllowOverlap,
  "Hit testing to allow subsequent widgets to overlap this one.");
API_ENUM(0_1, ImGui, TreeNodeFlags_NoTreePushOnOpen,
R"(Don't do a TreePush when open (e.g. for CollapsingHeader)
   = no extra indent nor pushing on ID stack.)");
API_ENUM(0_1, ImGui, TreeNodeFlags_NoAutoOpenOnLog,
R"(Don't automatically and temporarily open node when Logging is active
   (by default logging will automatically open tree nodes).)");
API_ENUM(0_1, ImGui, TreeNodeFlags_DefaultOpen,       "Default node to be open.");
API_ENUM(0_1, ImGui, TreeNodeFlags_OpenOnDoubleClick, "Need double-click to open node.");
API_ENUM(0_1, ImGui, TreeNodeFlags_OpenOnArrow,
R"(Only open when clicking on the arrow part.
   If TreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or
   double-click all box to open.)");
API_ENUM(0_1, ImGui, TreeNodeFlags_Leaf,
  "No collapsing, no arrow (use as a convenience for leaf nodes).");
API_ENUM(0_1, ImGui, TreeNodeFlags_Bullet,
R"(Display a bullet instead of arrow. IMPORTANT: node can still be marked
   open/close if you don't set the _Leaf flag!)");
API_ENUM(0_1, ImGui, TreeNodeFlags_FramePadding,
R"(Use FramePadding (even for an unframed text node) to vertically align text
   baseline to regular widget height.
   Equivalent to calling AlignTextToFramePadding before the node.)");
API_ENUM(0_1, ImGui, TreeNodeFlags_SpanAvailWidth,
R"(Extend hit box to the right-most edge, even if not framed.
   This is not the default in order to allow adding other items on the same line.
   In the future we may refactor the hit system to be front-to-back,
   allowing natural overlaps and then this can become the default.)");
API_ENUM(0_1, ImGui, TreeNodeFlags_SpanFullWidth,
  "Extend hit box to the left-most and right-most edges (bypass the indented area).");
API_ENUM(0_9_1, ImGui, TreeNodeFlags_SpanTextWidth,
  "Narrow hit box + narrow hovering highlight, will only cover the label text.");
API_ENUM(0_9, ImGui, TreeNodeFlags_SpanAllColumns,
  "Frame will span all columns of its container table (text will still fit in current column).");
// API_ENUM(ImGui, TreeNodeFlags_NavLeftJumpsBackHere,
// R"((WIP) Nav: left direction may move to this TreeNode() from any of its child
//    (items submitted between TreeNode and TreePop).)");
API_ENUM(0_1, ImGui, TreeNodeFlags_CollapsingHeader,
  "TreeNodeFlags_Framed | TreeNodeFlags_NoTreePushOnOpen | TreeNodeFlags_NoAutoOpenOnLog");
