/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
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

#include "api_helper.hpp"

#include "font.hpp"

#define DEFINE_ENUM(prefix, name, doc) \
  DEFINE_API(int, name, NO_ARGS, doc, { return prefix##name; })

// ImGuiCol
DEFINE_ENUM(ImGui, Col_Text,                  "");
DEFINE_ENUM(ImGui, Col_TextDisabled,          "");
DEFINE_ENUM(ImGui, Col_WindowBg,              "Background of normal windows.");
DEFINE_ENUM(ImGui, Col_ChildBg,               "Background of child windows.");
DEFINE_ENUM(ImGui, Col_PopupBg,               "Background of popups, menus, tooltips windows.");
DEFINE_ENUM(ImGui, Col_Border,                "");
DEFINE_ENUM(ImGui, Col_BorderShadow,          "");
DEFINE_ENUM(ImGui, Col_FrameBg,               "Background of checkbox, radio button, plot, slider, text input.");
DEFINE_ENUM(ImGui, Col_FrameBgHovered,        "");
DEFINE_ENUM(ImGui, Col_FrameBgActive,         "");
DEFINE_ENUM(ImGui, Col_TitleBg,               "");
DEFINE_ENUM(ImGui, Col_TitleBgActive,         "");
DEFINE_ENUM(ImGui, Col_TitleBgCollapsed,      "");
DEFINE_ENUM(ImGui, Col_MenuBarBg,             "");
DEFINE_ENUM(ImGui, Col_ScrollbarBg,           "");
DEFINE_ENUM(ImGui, Col_ScrollbarGrab,         "");
DEFINE_ENUM(ImGui, Col_ScrollbarGrabHovered,  "");
DEFINE_ENUM(ImGui, Col_ScrollbarGrabActive,   "");
DEFINE_ENUM(ImGui, Col_CheckMark,             "");
DEFINE_ENUM(ImGui, Col_SliderGrab,            "");
DEFINE_ENUM(ImGui, Col_SliderGrabActive,      "");
DEFINE_ENUM(ImGui, Col_Button,                "");
DEFINE_ENUM(ImGui, Col_ButtonHovered,         "");
DEFINE_ENUM(ImGui, Col_ButtonActive,          "");
DEFINE_ENUM(ImGui, Col_Header,                "Header* colors are used for ImGui_CollapsingHeader, ImGui_TreeNode, ImGui_Selectable, ImGui_MenuItem.");
DEFINE_ENUM(ImGui, Col_HeaderHovered,         "");
DEFINE_ENUM(ImGui, Col_HeaderActive,          "");
DEFINE_ENUM(ImGui, Col_Separator,             "");
DEFINE_ENUM(ImGui, Col_SeparatorHovered,      "");
DEFINE_ENUM(ImGui, Col_SeparatorActive,       "");
DEFINE_ENUM(ImGui, Col_ResizeGrip,            "");
DEFINE_ENUM(ImGui, Col_ResizeGripHovered,     "");
DEFINE_ENUM(ImGui, Col_ResizeGripActive,      "");
DEFINE_ENUM(ImGui, Col_Tab,                   "");
DEFINE_ENUM(ImGui, Col_TabHovered,            "");
DEFINE_ENUM(ImGui, Col_TabActive,             "");
DEFINE_ENUM(ImGui, Col_TabUnfocused,          "");
DEFINE_ENUM(ImGui, Col_TabUnfocusedActive,    "");
DEFINE_ENUM(ImGui, Col_DockingPreview,        "Preview overlay color when about to docking something.");
DEFINE_ENUM(ImGui, Col_DockingEmptyBg,        "Background color for empty node (e.g. CentralNode with no window docked into it).");
DEFINE_ENUM(ImGui, Col_PlotLines,             "");
DEFINE_ENUM(ImGui, Col_PlotLinesHovered,      "");
DEFINE_ENUM(ImGui, Col_PlotHistogram,         "");
DEFINE_ENUM(ImGui, Col_PlotHistogramHovered,  "");
DEFINE_ENUM(ImGui, Col_TableHeaderBg,         "Table header background.");
DEFINE_ENUM(ImGui, Col_TableBorderStrong,     "Table outer and header borders (prefer using Alpha=1.0 here).");
DEFINE_ENUM(ImGui, Col_TableBorderLight,      "Table inner borders (prefer using Alpha=1.0 here).");
DEFINE_ENUM(ImGui, Col_TableRowBg,            "Table row background (even rows).");
DEFINE_ENUM(ImGui, Col_TableRowBgAlt,         "Table row background (odd rows).");
DEFINE_ENUM(ImGui, Col_TextSelectedBg,        "");
DEFINE_ENUM(ImGui, Col_DragDropTarget,        "");
DEFINE_ENUM(ImGui, Col_NavHighlight,          "Gamepad/keyboard: current highlighted item.");
DEFINE_ENUM(ImGui, Col_NavWindowingHighlight, "Highlight window when using CTRL+TAB.");
DEFINE_ENUM(ImGui, Col_NavWindowingDimBg,     "Darken/colorize entire screen behind the CTRL+TAB window list, when active.");
DEFINE_ENUM(ImGui, Col_ModalWindowDimBg,      "Darken/colorize entire screen behind a modal window, when one is active.");

// ImGuiCond
DEFINE_ENUM(ImGui, Cond_Always,       "No condition (always set the variable).");
DEFINE_ENUM(ImGui, Cond_Once,         "Set the variable once per runtime session (only the first call will succeed).");
DEFINE_ENUM(ImGui, Cond_FirstUseEver, "Set the variable if the object/window has no persistently saved data (no entry in .ini file).");
DEFINE_ENUM(ImGui, Cond_Appearing,    "Set the variable if the object/window is appearing after being hidden/inactive (or the first time).");

// typedef int ImGuiDataType;          // -> enum ImGuiDataType_        // Enum: A primary data type

// ImGuiDir
DEFINE_ENUM(ImGui, Dir_None,  "A cardinal direction.");
DEFINE_ENUM(ImGui, Dir_Left,  "A cardinal direction.");
DEFINE_ENUM(ImGui, Dir_Right, "A cardinal direction.");
DEFINE_ENUM(ImGui, Dir_Up,    "A cardinal direction.");
DEFINE_ENUM(ImGui, Dir_Down,  "A cardinal direction.");

// typedef int ImGuiKey;               // -> enum ImGuiKey_             // Enum: A key identifier (ImGui-side enum)
// typedef int ImGuiNavInput;          // -> enum ImGuiNavInput_        // Enum: An input identifier for navigation

// ImGuiMouseButton
DEFINE_ENUM(ImGui, MouseButton_Left,   "");
DEFINE_ENUM(ImGui, MouseButton_Right,  "");
DEFINE_ENUM(ImGui, MouseButton_Middle, "");

// ImGuiMouseCursor
// DEFINE_ENUM(ImGui, MouseCursor_None,       "");
DEFINE_ENUM(ImGui, MouseCursor_Arrow,      "");
DEFINE_ENUM(ImGui, MouseCursor_TextInput,  "When hovering over ImGui_InputText, etc.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeAll,  "(Unused by Dear ImGui functions)");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNS,   "When hovering over an horizontal border.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeEW,   "When hovering over a vertical border or a column.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNESW, "When hovering over the bottom-left corner of a window.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNWSE, "When hovering over the bottom-right corner of a window.");
DEFINE_ENUM(ImGui, MouseCursor_Hand,       "(Unused by Dear ImGui functions. Use for e.g. hyperlinks)");
DEFINE_ENUM(ImGui, MouseCursor_NotAllowed, "When hovering something with disallowed interaction. Usually a crossed circle.");

// ImGuiSortDirection
DEFINE_ENUM(ImGui, SortDirection_None,       "");
DEFINE_ENUM(ImGui, SortDirection_Ascending,  "Ascending = 0->9, A->Z etc.");
DEFINE_ENUM(ImGui, SortDirection_Descending, "Descending = 9->0, Z->A etc.");

// ImGuiStyleVar
DEFINE_ENUM(ImGui, StyleVar_Alpha,               "Global alpha applies to everything in Dear ImGui.");
DEFINE_ENUM(ImGui, StyleVar_WindowPadding,       "Padding within a window.");
DEFINE_ENUM(ImGui, StyleVar_WindowRounding,      "Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.");
DEFINE_ENUM(ImGui, StyleVar_WindowBorderSize,    "Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_WindowMinSize,       "Minimum window size. This is a global setting. If you want to constraint individual windows, use ImGui_SetNextWindowSizeConstraints.");
DEFINE_ENUM(ImGui, StyleVar_WindowTitleAlign,    "Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.");
DEFINE_ENUM(ImGui, StyleVar_ChildRounding,       "Radius of child window corners rounding. Set to 0.0f to have rectangular windows.");
DEFINE_ENUM(ImGui, StyleVar_ChildBorderSize,     "Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_PopupRounding,       "Radius of popup window corners rounding. (Note that tooltip windows use ImGui_StyleVar_WindowRounding.)");
DEFINE_ENUM(ImGui, StyleVar_PopupBorderSize,     "Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_FramePadding,        "Padding within a framed rectangle (used by most widgets).");
DEFINE_ENUM(ImGui, StyleVar_FrameRounding,       "Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).");
DEFINE_ENUM(ImGui, StyleVar_FrameBorderSize,     "Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_ItemSpacing,         "Horizontal and vertical spacing between widgets/lines.");
DEFINE_ENUM(ImGui, StyleVar_ItemInnerSpacing,    "Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).");
DEFINE_ENUM(ImGui, StyleVar_IndentSpacing,       "Horizontal indentation when e.g. entering a tree node. Generally == (ImGui_GetFontSize + ImGui_StyleVar_FramePadding.x*2).");
DEFINE_ENUM(ImGui, StyleVar_CellPadding,         "Padding within a table cell.");
DEFINE_ENUM(ImGui, StyleVar_ScrollbarSize,       "Width of the vertical scrollbar, Height of the horizontal scrollbar.");
DEFINE_ENUM(ImGui, StyleVar_ScrollbarRounding,   "Radius of grab corners for scrollbar.");
DEFINE_ENUM(ImGui, StyleVar_GrabMinSize,         "Minimum width/height of a grab box for slider/scrollbar.");
DEFINE_ENUM(ImGui, StyleVar_GrabRounding,        "Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.");
DEFINE_ENUM(ImGui, StyleVar_TabRounding,         "Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.");
DEFINE_ENUM(ImGui, StyleVar_ButtonTextAlign,     "Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).");
DEFINE_ENUM(ImGui, StyleVar_SelectableTextAlign, "Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.");

// ImGuiTableBgTarget
DEFINE_ENUM(ImGui, TableBgTarget_None, R"(Enum: A color target for TableSetBgColor()

Background colors are rendering in 3 layers:
 - Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if set.
 - Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if set.
 - Layer 2: draw with CellBg color if set.

The purpose of the two row/columns layers is to let you decide if a background color changes should override or blend with the existing color.
When using ImGui_TableFlags_RowBg on the table, each row has the RowBg0 color automatically set for odd/even rows.
If you set the color of RowBg0 target, your color will override the existing RowBg0 color.
If you set the color of RowBg1 or ColumnBg1 target, your color will blend over the RowBg0 color.)");
DEFINE_ENUM(ImGui, TableBgTarget_RowBg0, "Set row background color 0 (generally used for background, automatically set when ImGui_TableFlags_RowBg is used).");
DEFINE_ENUM(ImGui, TableBgTarget_RowBg1, "Set row background color 1 (generally used for selection marking).");
DEFINE_ENUM(ImGui, TableBgTarget_CellBg, "Set cell background color (top-most color).");

// ImDrawFlags
DEFINE_ENUM(Im, DrawFlags_None,                         "");
DEFINE_ENUM(Im, DrawFlags_Closed,                       "ImGui_DrawList_PathStroke, ImGui_DrawList_AddPolyline: specify that shape should be closed (Important: this is always == 1 for legacy reason).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTopLeft,          "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding top-left corner only (when rounding > 0.0f, we default to all corners).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTopRight,         "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding top-right corner only (when rounding > 0.0f, we default to all corners).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottomLeft,       "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottomRight,      "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners).");

DEFINE_ENUM(Im, DrawFlags_RoundCornersNone            , "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!.");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTop             , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottom          , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersLeft            , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersRight           , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersAll             , "");

// ImGuiButtonFlags
DEFINE_ENUM(ImGui, ButtonFlags_None,              "Flags for ImGui_InvisibleButton.");
DEFINE_ENUM(ImGui, ButtonFlags_MouseButtonLeft,   "React on left mouse button (default).");
DEFINE_ENUM(ImGui, ButtonFlags_MouseButtonRight,  "React on right mouse button.");
DEFINE_ENUM(ImGui, ButtonFlags_MouseButtonMiddle, "React on center mouse button.");

// ImGuiColorEditFlags
DEFINE_ENUM(ImGui, ColorEditFlags_None,             "Flags for ImGui_ColorEdit3 / ImGui_ColorEdit4 / ImGui_ColorPicker3 / ImGui_ColorPicker4 / ImGui_ColorButton.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoAlpha,          "ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoPicker,         "ColorEdit: disable picker when clicking on color square.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoOptions,        "ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoSmallPreview,   "ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoInputs,         "ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoTooltip,        "ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoLabel,          "ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoSidePreview,    "ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoDragDrop,       "ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoBorder,         "ColorButton: disable border (which is enforced by default).");
// User Options (right-click on widget to change some of them).
DEFINE_ENUM(ImGui, ColorEditFlags_AlphaBar,         "ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.");
DEFINE_ENUM(ImGui, ColorEditFlags_AlphaPreview,     "ColorEdit, ColorPicker, ColorButton: display preview as a transparent color over a checkerboard, instead of opaque.");
DEFINE_ENUM(ImGui, ColorEditFlags_AlphaPreviewHalf, "ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard, instead of opaque.");
// DEFINE_ENUM(ImGui, ColorEditFlags_HDR,              "(WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use ImGuiColorEditFlags_Float flag as well).");
DEFINE_ENUM(ImGui, ColorEditFlags_DisplayRGB,       "ColorEdit: override _display_ type to RGB. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ImGui, ColorEditFlags_DisplayHSV,       "ColorEdit: override _display_ type to HSV. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ImGui, ColorEditFlags_DisplayHex,       "ColorEdit: override _display_ type to Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ImGui, ColorEditFlags_Uint8,            "ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.");
DEFINE_ENUM(ImGui, ColorEditFlags_Float,            "ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.");
DEFINE_ENUM(ImGui, ColorEditFlags_PickerHueBar,     "ColorPicker: bar for Hue, rectangle for Sat/Value.");
DEFINE_ENUM(ImGui, ColorEditFlags_PickerHueWheel,   "ColorPicker: wheel for Hue, triangle for Sat/Value.");
DEFINE_ENUM(ImGui, ColorEditFlags_InputRGB,         "ColorEdit, ColorPicker: input and output data in RGB format.");
DEFINE_ENUM(ImGui, ColorEditFlags_InputHSV,         "ColorEdit, ColorPicker: input and output data in HSV format.");
DEFINE_ENUM(ImGui, ColorEditFlags__OptionsDefault,  "Defaults Options. You can set application defaults using ImGui_SetColorEditOptions. The intent is that you probably don't want to override them in most of your calls. Let the user choose via the option menu and/or call SetColorEditOptions() once during startup.");

// ImGuiConfigFlags
DEFINE_ENUM(ImGui, ConfigFlags_None,                 "Flags for ImGui_SetConfigFlags.");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableKeyboard,    "Master keyboard navigation enable flag.");
// DEFINE_ENUM(ImGui, ConfigFlags_NavEnableGamepad,     "Master gamepad navigation enable flag. This is mostly to instruct your imgui backend to fill io.NavInputs[]. Backend also needs to set ImGuiBackendFlags_HasGamepad.");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableSetMousePos, "Instruct navigation to move the mouse cursor.");
// DEFINE_ENUM(ImGui, ConfigFlags_NavNoCaptureKeyboard, "Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouse,              "Instruct imgui to ignore mouse position/buttons.");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouseCursorChange,  "Instruct backend to not alter mouse cursor shape and visibility.");
DEFINE_ENUM(ImGui, ConfigFlags_DockingEnable,        "[BETA] Enable docking functionality.");

// ImGuiComboFlags
DEFINE_ENUM(ImGui, ComboFlags_None,           "Flags for ImGui_BeginCombo.");
DEFINE_ENUM(ImGui, ComboFlags_PopupAlignLeft, "Align the popup toward the left by default.");
DEFINE_ENUM(ImGui, ComboFlags_HeightSmall,    "Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use ImGui_SetNextWindowSizeConstraints prior to calling ImGui_BeginCombo.");
DEFINE_ENUM(ImGui, ComboFlags_HeightRegular,  "Max ~8 items visible (default).");
DEFINE_ENUM(ImGui, ComboFlags_HeightLarge,    "Max ~20 items visible.");
DEFINE_ENUM(ImGui, ComboFlags_HeightLargest,  "As many fitting items as possible.");
DEFINE_ENUM(ImGui, ComboFlags_NoArrowButton,  "Display on the preview box without the square arrow button.");
DEFINE_ENUM(ImGui, ComboFlags_NoPreview,      "Display only a square arrow button.");

// ImGuiDragDropFlags
DEFINE_ENUM(ImGui, DragDropFlags_None,                     "Flags for ImGui_BeginDragDropSource, ImGui_AcceptDragDropPayload.");
// BeginDragDropSource() flags
DEFINE_ENUM(ImGui, DragDropFlags_SourceNoPreviewTooltip,   "By default, a successful call to ImGui_BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disable this behavior.");
DEFINE_ENUM(ImGui, DragDropFlags_SourceNoDisableHover,     "By default, when dragging we clear data so that ImGui_IsItemHovered will return false, to avoid subsequent user code submitting tooltips. This flag disable this behavior so you can still call ImGui_IsItemHovered on the source item.");
DEFINE_ENUM(ImGui, DragDropFlags_SourceNoHoldToOpenOthers, "Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.");
DEFINE_ENUM(ImGui, DragDropFlags_SourceAllowNullID,        "Allow items such as ImGui_Text, ImGui_Image that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear imgui ecosystem and so we made it explicit.");
DEFINE_ENUM(ImGui, DragDropFlags_SourceExtern,             "External source (from outside of dear imgui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.");
DEFINE_ENUM(ImGui, DragDropFlags_SourceAutoExpirePayload,  "Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged).");
// AcceptDragDropPayload() flags
DEFINE_ENUM(ImGui, DragDropFlags_AcceptBeforeDelivery,     "ImGui_AcceptDragDropPayload will returns true even before the mouse button is released. You can then check ImGui_GetDragDropPayload/is_delivery to test if the payload needs to be delivered.");
DEFINE_ENUM(ImGui, DragDropFlags_AcceptNoDrawDefaultRect,  "Do not draw the default highlight rectangle when hovering over target.");
DEFINE_ENUM(ImGui, DragDropFlags_AcceptNoPreviewTooltip,   "Request hiding the ImGui_BeginDragDropSource tooltip from the ImGui_BeginDragDropTarget site.");
DEFINE_ENUM(ImGui, DragDropFlags_AcceptPeekOnly,           "For peeking ahead and inspecting the payload before delivery. Equivalent to ImGui_DragDropFlags_AcceptBeforeDelivery | ImGui_DragDropFlags_AcceptNoDrawDefaultRect.");

// ImGuiFocusedFlags
DEFINE_ENUM(ImGui, FocusedFlags_None,                "Flags for ImGui_IsWindowFocused.");
DEFINE_ENUM(ImGui, FocusedFlags_ChildWindows,        "ImGui_IsWindowFocused: Return true if any children of the window is focused.");
DEFINE_ENUM(ImGui, FocusedFlags_RootWindow,          "ImGui_IsWindowFocused: Test from root window (top most parent of the current hierarchy).");
DEFINE_ENUM(ImGui, FocusedFlags_AnyWindow,           "ImGui_IsWindowFocused: Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!.");
DEFINE_ENUM(ImGui, FocusedFlags_RootAndChildWindows, "ImGui_FocusedFlags_RootWindow | ImGui_FocusedFlags_ChildWindows");

// ImGuiHoveredFlags
// Flags: for IsItemHovered(), IsWindowHovered() etc.
DEFINE_ENUM(ImGui, HoveredFlags_None,                         "Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.");
DEFINE_ENUM(ImGui, HoveredFlags_ChildWindows,                 "ImGui_IsWindowHovered only: Return true if any children of the window is hovered.");
DEFINE_ENUM(ImGui, HoveredFlags_RootWindow,                   "ImGui_IsWindowHovered only: Test from root window (top most parent of the current hierarchy).");
DEFINE_ENUM(ImGui, HoveredFlags_AnyWindow,                    "ImGui_IsWindowHovered only: Return true if any window is hovered.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenBlockedByPopup,      "Return true even if a popup window is normally blocking access to this item/window.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenBlockedByActiveItem, "Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenOverlapped,          "Return true even if the position is obstructed or overlapped by another window.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenDisabled,            "Return true even if the item is disabled.");
DEFINE_ENUM(ImGui, HoveredFlags_RectOnly,                     "ImGui_HoveredFlags_AllowWhenBlockedByPopup | ImGui_HoveredFlags_AllowWhenBlockedByActiveItem | ImGui_HoveredFlags_AllowWhenOverlapped");
DEFINE_ENUM(ImGui, HoveredFlags_RootAndChildWindows,          "ImGui_HoveredFlags_RootWindow | ImGui_HoveredFlags_ChildWindows");

// ImGuiInputTextFlags
DEFINE_ENUM(ImGui, InputTextFlags_None,                "Most of the InputTextFlags flags are only useful for ImGui_InputText and not for InputIntX, InputDouble etc.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsDecimal,        "Allow 0123456789.+-*/.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsHexadecimal,    "Allow 0123456789ABCDEFabcdef.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsUppercase,      "Turn a..z into A..Z.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsNoBlank,        "Filter out spaces, tabs.");
DEFINE_ENUM(ImGui, InputTextFlags_AutoSelectAll,       "Select entire text when first taking mouse focus.");
DEFINE_ENUM(ImGui, InputTextFlags_EnterReturnsTrue,    "Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the ImGui_IsItemDeactivatedAfterEdit function.");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackCompletion,  "Callback on pressing TAB (for completion handling).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackHistory,     "Callback on pressing Up/Down arrows (for history handling).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackAlways,      "Callback on each iteration. User code may query cursor position, modify text buffer.");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackCharFilter,  "Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.");
DEFINE_ENUM(ImGui, InputTextFlags_AllowTabInput,       "Pressing TAB input a '\\t' character into the text field.");
DEFINE_ENUM(ImGui, InputTextFlags_CtrlEnterForNewLine, "In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).");
DEFINE_ENUM(ImGui, InputTextFlags_NoHorizontalScroll,  "Disable following the cursor horizontally.");
DEFINE_ENUM(ImGui, InputTextFlags_AlwaysOverwrite,     "Overwrite mode.");
DEFINE_ENUM(ImGui, InputTextFlags_ReadOnly,            "Read-only mode.");
DEFINE_ENUM(ImGui, InputTextFlags_Password,            "Password mode, display all characters as '*'.");
DEFINE_ENUM(ImGui, InputTextFlags_NoUndoRedo,          "Disable undo/redo. Note that input text owns the text data while active.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsScientific,     "Allow 0123456789.+-*/eE (Scientific notation input).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackResize,      "Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackEdit,        "Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active).");
// [Internal]
// DEFINE_ENUM(ImGui, InputTextFlags_Multiline,           "For internal use by InputTextMultiline().");
// DEFINE_ENUM(ImGui, InputTextFlags_NoMarkEdited,        "For internal use by functions using InputText() before reformatting data.");

// ImGuiKeyModFlags
DEFINE_ENUM(ImGui, KeyModFlags_None,  "");
DEFINE_ENUM(ImGui, KeyModFlags_Ctrl,  "");
DEFINE_ENUM(ImGui, KeyModFlags_Shift, "");
DEFINE_ENUM(ImGui, KeyModFlags_Alt,   "");
DEFINE_ENUM(ImGui, KeyModFlags_Super, "");


// ImGuiPopupFlags
DEFINE_ENUM(ImGui, PopupFlags_None,                    "Flags for OpenPopup*(), BeginPopupContext*(), ImGui_IsPopupOpen.");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonLeft,         "For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as ImGui_MouseButton_Left).");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonRight,        "For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as ImGui_MouseButton_Right).");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonMiddle,       "For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as ImGui_MouseButton_Middle).");
DEFINE_ENUM(ImGui, PopupFlags_NoOpenOverExistingPopup, "For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack.");
DEFINE_ENUM(ImGui, PopupFlags_NoOpenOverItems,         "For ImGui_BeginPopupContextWindow: don't return true when hovering items, only when hovering empty space.");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopupId,              "For ImGui_IsPopupOpen: ignore the str_id parameter and test for any popup.");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopupLevel,           "For ImGui_IsPopupOpen: search/test at any level of the popup stack (default test in the current level).");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopup,                "ImGui_PopupFlags_AnyPopupId | ImGui_PopupFlags_AnyPopupLevel");

// ImGuiSelectableFlags
DEFINE_ENUM(ImGui, SelectableFlags_None,             "Flags for ImGui_Selectable.");
DEFINE_ENUM(ImGui, SelectableFlags_DontClosePopups,  "Clicking this don't close parent popup window.");
DEFINE_ENUM(ImGui, SelectableFlags_SpanAllColumns,   "Selectable frame can span all columns (text will still fit in current column).");
DEFINE_ENUM(ImGui, SelectableFlags_AllowDoubleClick, "Generate press events on double clicks too.");
DEFINE_ENUM(ImGui, SelectableFlags_Disabled,         "Cannot be selected, display grayed out text.");
DEFINE_ENUM(ImGui, SelectableFlags_AllowItemOverlap, "Hit testing to allow subsequent widgets to overlap this one.");

// ImGuiSliderFlags
DEFINE_ENUM(ImGui, SliderFlags_None,            "For ImGui_DragDouble, ImGui_DragInt, ImGui_SliderDouble, ImGui_SliderInt etc.");
DEFINE_ENUM(ImGui, SliderFlags_AlwaysClamp,     "Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.");
DEFINE_ENUM(ImGui, SliderFlags_Logarithmic,     "Make the widget logarithmic (linear otherwise). Consider using ImGui_SliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.");
DEFINE_ENUM(ImGui, SliderFlags_NoRoundToFormat, "Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits).");
DEFINE_ENUM(ImGui, SliderFlags_NoInput,         "Disable CTRL+Click or Enter key allowing to input text directly into the widget.");

// ImGuiTabBarFlags
DEFINE_ENUM(ImGui, TabBarFlags_None,                         "Flags for ImGui_BeginTabBar.");
DEFINE_ENUM(ImGui, TabBarFlags_Reorderable,                  "Allow manually dragging tabs to re-order them + New tabs are appended at the end of list.");
DEFINE_ENUM(ImGui, TabBarFlags_AutoSelectNewTabs,            "Automatically select new tabs when they appear.");
DEFINE_ENUM(ImGui, TabBarFlags_TabListPopupButton,           "Disable buttons to open the tab list popup.");
DEFINE_ENUM(ImGui, TabBarFlags_NoCloseWithMiddleMouseButton, "Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if(ImGui_IsItemHovered() && ImGui_IsMouseClicked(2)) *p_open = false.");
DEFINE_ENUM(ImGui, TabBarFlags_NoTabListScrollingButtons,    "Disable scrolling buttons (apply when fitting policy is ImGui_TabBarFlags_FittingPolicyScroll).");
DEFINE_ENUM(ImGui, TabBarFlags_NoTooltip,                    "Disable tooltips when hovering a tab.");
DEFINE_ENUM(ImGui, TabBarFlags_FittingPolicyResizeDown,      "Resize tabs when they don't fit.");
DEFINE_ENUM(ImGui, TabBarFlags_FittingPolicyScroll,          "Add scroll buttons when tabs don't fit.");

// ImGuiTabItemFlags
DEFINE_ENUM(ImGui, TabItemFlags_None,                         "Flags for ImGui_BeginTabItem.");
DEFINE_ENUM(ImGui, TabItemFlags_UnsavedDocument,              "Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. Also: tab is selected on closure and closure is deferred by one frame to allow code to undo it without flicker.");
DEFINE_ENUM(ImGui, TabItemFlags_SetSelected,                  "Trigger flag to programmatically make the tab selected when calling ImGui_BeginTabItem.");
DEFINE_ENUM(ImGui, TabItemFlags_NoCloseWithMiddleMouseButton, "Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (ImGui_IsItemHovered() && ImGui_IsMouseClicked(2)) *p_open = false.");
DEFINE_ENUM(ImGui, TabItemFlags_NoPushId,                     "Don't call ImGui_PushID(tab->ID)/ImGui_PopID() on ImGui_BeginTabItem/ImGui_EndTabItem.");
DEFINE_ENUM(ImGui, TabItemFlags_NoTooltip,                    "Disable tooltip for the given tab.");
DEFINE_ENUM(ImGui, TabItemFlags_NoReorder,                    "Disable reordering this tab or having another tab cross over this tab.");
DEFINE_ENUM(ImGui, TabItemFlags_Leading,                      "Enforce the tab position to the left of the tab bar (after the tab list popup button).");
DEFINE_ENUM(ImGui, TabItemFlags_Trailing,                     "Enforce the tab position to the right of the tab bar (before the scrolling buttons).");

// ImGuiTableFlags
DEFINE_ENUM(ImGui, TableFlags_None, R"(For ImGui_BeginTable.

- Important! Sizing policies have complex and subtle side effects, more so than you would expect.
  Read comments/demos carefully + experiment with live demos to get acquainted with them.
- The DEFAULT sizing policies are:
   - Default to ImGui_TableFlags_SizingFixedFit    if ScrollX is on, or if host window has ImGui_WindowFlags_AlwaysAutoResize.
   - Default to ImGui_TableFlags_SizingStretchSame if ScrollX is off.
- When ScrollX is off:
   - Table defaults to ImGui_TableFlags_SizingStretchSame -> all Columns defaults to ImGui_TableColumnFlags_WidthStretch with same weight.
   - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
   - Fixed Columns will generally obtain their requested width (unless the table cannot fit them all).
   - Stretch Columns will share the remaining width.
   - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
     The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
     (this is because the visible order of columns have subtle but necessary effects on how they react to manual resizing).
- When ScrollX is on:
   - Table defaults to ImGui_TableFlags_SizingFixedFit -> all Columns defaults to ImGui_TableColumnFlags_WidthFixed
   - Columns sizing policy allowed: Fixed/Auto mostly.
   - Fixed Columns can be enlarged as needed. Table will show an horizontal scrollbar if needed.
   - When using auto-resizing (non-resizable) fixed columns, querying the content width to use item right-alignment e.g. ImGui_SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a feedback loop.
   - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS you have specified a value for 'inner_width' in ImGui_BeginTable().
     If you specify a value for 'inner_width' then effectively the scrolling space is known and Stretch or mixed Fixed/Stretch columns become meaningful again.
- Read on documentation at the top of imgui_tables.cpp for details.)");
// Features
DEFINE_ENUM(ImGui, TableFlags_Resizable,                  "Enable resizing columns.");
DEFINE_ENUM(ImGui, TableFlags_Reorderable,                "Enable reordering columns in header row (need calling ImGui_TableSetupColumn + ImGui_TableHeadersRow to display headers).");
DEFINE_ENUM(ImGui, TableFlags_Hideable,                   "Enable hiding/disabling columns in context menu.");
DEFINE_ENUM(ImGui, TableFlags_Sortable,                   "Enable sorting. Call ImGui_TableNeedSort/ImGui_TableGetColumnSortSpecs to obtain sort specs. Also see ImGui_TableFlags_SortMulti and ImGui_TableFlags_SortTristate.");
DEFINE_ENUM(ImGui, TableFlags_NoSavedSettings,            "Disable persisting columns order, width and sort settings in the .ini file.");
DEFINE_ENUM(ImGui, TableFlags_ContextMenuInBody,          "Right-click on columns body/contents will display table context menu. By default it is available in ImGui_TableHeadersRow.");
// Decorations
DEFINE_ENUM(ImGui, TableFlags_RowBg,                      "Set each RowBg color with ImGui_Col_TableRowBg or ImGui_Col_TableRowBgAlt (equivalent of calling ImGui_TableSetBgColor with ImGui_TableBgTarget_RowBg0 on each row manually).");
DEFINE_ENUM(ImGui, TableFlags_BordersInnerH,              "Draw horizontal borders between rows.");
DEFINE_ENUM(ImGui, TableFlags_BordersOuterH,              "Draw horizontal borders at the top and bottom.");
DEFINE_ENUM(ImGui, TableFlags_BordersInnerV,              "Draw vertical borders between columns.");
DEFINE_ENUM(ImGui, TableFlags_BordersOuterV,              "Draw vertical borders on the left and right sides.");
DEFINE_ENUM(ImGui, TableFlags_BordersH,                   "Draw horizontal borders.");
DEFINE_ENUM(ImGui, TableFlags_BordersV,                   "Draw vertical borders.");
DEFINE_ENUM(ImGui, TableFlags_BordersInner,               "Draw inner borders.");
DEFINE_ENUM(ImGui, TableFlags_BordersOuter,               "Draw outer borders.");
DEFINE_ENUM(ImGui, TableFlags_Borders,                    "Draw all borders.");
// DEFINE_ENUM(ImGui, TableFlags_NoBordersInBody,            "[ALPHA] Disable vertical borders in columns Body (borders will always appears in Headers). -> May move to style.");
// DEFINE_ENUM(ImGui, TableFlags_NoBordersInBodyUntilResize, "[ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appears in Headers). -> May move to style.");
// Sizing Policy (read above for defaults)
DEFINE_ENUM(ImGui, TableFlags_SizingFixedFit,             "Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.");
DEFINE_ENUM(ImGui, TableFlags_SizingFixedSame,            "Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable ImGui_TableFlags_NoKeepColumnsVisible.");
DEFINE_ENUM(ImGui, TableFlags_SizingStretchProp,          "Columns default to _WidthStretch with default weights proportional to each columns contents widths.");
DEFINE_ENUM(ImGui, TableFlags_SizingStretchSame,          "Columns default to _WidthStretch with default weights all equal, unless overriden by ImGui_TableSetupColumn.");
// Sizing Extra Options
DEFINE_ENUM(ImGui, TableFlags_NoHostExtendX,              "Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
DEFINE_ENUM(ImGui, TableFlags_NoHostExtendY,              "Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
DEFINE_ENUM(ImGui, TableFlags_NoKeepColumnsVisible,       "Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.");
DEFINE_ENUM(ImGui, TableFlags_PreciseWidths,              "Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
// Clipping
DEFINE_ENUM(ImGui, TableFlags_NoClip,                     "Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with ImGui_TableSetupScrollFreeze.");
// Padding
DEFINE_ENUM(ImGui, TableFlags_PadOuterX,                  "Default if ImGui_TableFlags_BordersOuterV is on. Enable outer-most padding. Generally desirable if you have headers.");
DEFINE_ENUM(ImGui, TableFlags_NoPadOuterX,                "Default if ImGui_TableFlags_BordersOuterV is off. Disable outer-most padding.");
DEFINE_ENUM(ImGui, TableFlags_NoPadInnerX,                "Disable inner padding between columns (double inner padding if ImGui_TableFlags_BordersOuterV is on, single inner padding if BordersOuterV is off).");
// Scrolling
DEFINE_ENUM(ImGui, TableFlags_ScrollX,                    "Enable horizontal scrolling. Require 'outer_size' parameter of ImGui_BeginTable to specify the container size. Changes default sizing policy. Because this create a child window, ScrollY is currently generally recommended when using ScrollX.");
DEFINE_ENUM(ImGui, TableFlags_ScrollY,                    "Enable vertical scrolling. Require 'outer_size' parameter of ImGui_BeginTable to specify the container size.");
// Sorting
DEFINE_ENUM(ImGui, TableFlags_SortMulti,                  "Hold shift when clicking headers to sort on multiple column. ImGui_TableGetGetSortSpecs may return specs where (SpecsCount > 1).");
DEFINE_ENUM(ImGui, TableFlags_SortTristate,               "Allow no sorting, disable default sorting. ImGui_TableGetColumnSortSpecs may return specs where (SpecsCount == 0).");

// ImGuiTableColumnFlags
DEFINE_ENUM(ImGui, TableColumnFlags_None,                 "Flags for ImGui_TableSetupColumn.");
// Input configuration flags
DEFINE_ENUM(ImGui, TableColumnFlags_DefaultHide,          "Default as a hidden/disabled column.");
DEFINE_ENUM(ImGui, TableColumnFlags_DefaultSort,          "Default as a sorting column.");
DEFINE_ENUM(ImGui, TableColumnFlags_WidthStretch,         "Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).");
DEFINE_ENUM(ImGui, TableColumnFlags_WidthFixed,           "Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).");
DEFINE_ENUM(ImGui, TableColumnFlags_NoResize,             "Disable manual resizing.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoReorder,            "Disable manual reordering this column, this will also prevent other columns from crossing over this column.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoHide,               "Disable ability to hide/disable this column.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoClip,               "Disable clipping for this column (all NoClip columns will render in a same draw command).");
DEFINE_ENUM(ImGui, TableColumnFlags_NoSort,               "Disable ability to sort on this field (even if ImGui_TableFlags_Sortable is set on the table).");
DEFINE_ENUM(ImGui, TableColumnFlags_NoSortAscending,      "Disable ability to sort in the ascending direction.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoSortDescending,     "Disable ability to sort in the descending direction.");
DEFINE_ENUM(ImGui, TableColumnFlags_NoHeaderWidth,        "Disable header text width contribution to automatic column width.");
DEFINE_ENUM(ImGui, TableColumnFlags_PreferSortAscending,  "Make the initial sort direction Ascending when first sorting on this column (default).");
DEFINE_ENUM(ImGui, TableColumnFlags_PreferSortDescending, "Make the initial sort direction Descending when first sorting on this column.");
DEFINE_ENUM(ImGui, TableColumnFlags_IndentEnable,         "Use current Indent value when entering cell (default for column 0).");
DEFINE_ENUM(ImGui, TableColumnFlags_IndentDisable,        "Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.");
// Output status flags, read-only via TableGetColumnFlags()
DEFINE_ENUM(ImGui, TableColumnFlags_IsEnabled,            "Status: is enabled == not hidden by user/api (referred to as \"Hide\" in _DefaultHide and _NoHide) flags.");
DEFINE_ENUM(ImGui, TableColumnFlags_IsVisible,            "Status: is visible == is enabled AND not clipped by scrolling.");
DEFINE_ENUM(ImGui, TableColumnFlags_IsSorted,             "Status: is currently part of the sort specs.");
DEFINE_ENUM(ImGui, TableColumnFlags_IsHovered,            "Status: is hovered by mouse.");

// ImGuiTableRowFlags
DEFINE_ENUM(ImGui, TableRowFlags_None,    "Flags for ImGui_TableNextRow.");
DEFINE_ENUM(ImGui, TableRowFlags_Headers, "Identify header row (set default background color + width of its contents accounted different for auto column width).");

// ImGuiTreeNodeFlags
DEFINE_ENUM(ImGui, TreeNodeFlags_None,                 "Flags for ImGui_TreeNode, ImGui_TreeNodeEx, ImGui_CollapsingHeader.");
DEFINE_ENUM(ImGui, TreeNodeFlags_Selected,             "Draw as selected.");
DEFINE_ENUM(ImGui, TreeNodeFlags_Framed,               "Draw frame with background (e.g. for ImGui_CollapsingHeader).");
DEFINE_ENUM(ImGui, TreeNodeFlags_AllowItemOverlap,     "Hit testing to allow subsequent widgets to overlap this one.");
DEFINE_ENUM(ImGui, TreeNodeFlags_NoTreePushOnOpen,     "Don't do a ImGui_TreePush when open (e.g. for ImGui_CollapsingHeader) = no extra indent nor pushing on ID stack.");
DEFINE_ENUM(ImGui, TreeNodeFlags_NoAutoOpenOnLog,      "Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes).");
DEFINE_ENUM(ImGui, TreeNodeFlags_DefaultOpen,          "Default node to be open.");
DEFINE_ENUM(ImGui, TreeNodeFlags_OpenOnDoubleClick,    "Need double-click to open node.");
DEFINE_ENUM(ImGui, TreeNodeFlags_OpenOnArrow,          "Only open when clicking on the arrow part. If ImGui_TreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.");
DEFINE_ENUM(ImGui, TreeNodeFlags_Leaf,                 "No collapsing, no arrow (use as a convenience for leaf nodes).");
DEFINE_ENUM(ImGui, TreeNodeFlags_Bullet,               "Display a bullet instead of arrow.");
DEFINE_ENUM(ImGui, TreeNodeFlags_FramePadding,         "Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling ImGui_AlignTextToFramePadding.");
DEFINE_ENUM(ImGui, TreeNodeFlags_SpanAvailWidth,       "Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.");
DEFINE_ENUM(ImGui, TreeNodeFlags_SpanFullWidth,        "Extend hit box to the left-most and right-most edges (bypass the indented area).");
// DEFINE_ENUM(ImGui, TreeNodeFlags_NavLeftJumpsBackHere, "(WIP) Nav: left direction may move to this TreeNode() from any of its child (items submitted between TreeNode and TreePop).");
DEFINE_ENUM(ImGui, TreeNodeFlags_CollapsingHeader,     "");

// ImGuiWindowFlags
// for Begin(), BeginChild()
DEFINE_ENUM(ImGui, WindowFlags_None,                      "Default flag. See ImGui_Begin.");
DEFINE_ENUM(ImGui, WindowFlags_NoTitleBar,                "Disable title-bar.");
DEFINE_ENUM(ImGui, WindowFlags_NoResize,                  "Disable user resizing with the lower-right grip.");
DEFINE_ENUM(ImGui, WindowFlags_NoMove,                    "Disable user moving the window.");
DEFINE_ENUM(ImGui, WindowFlags_NoScrollbar,               "Disable scrollbars (window can still scroll with mouse or programmatically).");
DEFINE_ENUM(ImGui, WindowFlags_NoScrollWithMouse,         "Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.");
DEFINE_ENUM(ImGui, WindowFlags_NoCollapse,                "Disable user collapsing window by double-clicking on it.");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysAutoResize,          "Resize every window to its content every frame.");
DEFINE_ENUM(ImGui, WindowFlags_NoBackground,              "Disable drawing background color (WindowBg, etc.) and outside border. Similar as using ImGui_SetNextWindowBgAlpha(0.0).");
DEFINE_ENUM(ImGui, WindowFlags_NoSavedSettings,           "Never load/save settings in .ini file.");
DEFINE_ENUM(ImGui, WindowFlags_NoMouseInputs,             "Disable catching mouse, hovering test with pass through.");
DEFINE_ENUM(ImGui, WindowFlags_MenuBar,                   "Has a menu-bar.");
DEFINE_ENUM(ImGui, WindowFlags_HorizontalScrollbar,     R"(Allow horizontal scrollbar to appear (off by default). You may use ImGui_SetNextWindowContentSize(width, 0.0) prior to calling ImGui_Begin() to specify width. Read code in the demo's "Horizontal Scrolling" section.)");
DEFINE_ENUM(ImGui, WindowFlags_NoFocusOnAppearing,        "Disable taking focus when transitioning from hidden to visible state.");
DEFINE_ENUM(ImGui, WindowFlags_NoBringToFrontOnFocus,     "Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus).");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysVerticalScrollbar,   "Always show vertical scrollbar (even if ContentSize.y < Size.y).");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysHorizontalScrollbar, "Always show horizontal scrollbar (even if ContentSize.x < Size.x).");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysUseWindowPadding,    "Ensure child windows without border uses ImGui_StyleVar_WindowPadding (ignored by default for non-bordered child windows, because more convenient).");
DEFINE_ENUM(ImGui, WindowFlags_NoNavInputs,               "No gamepad/keyboard navigation within the window.");
DEFINE_ENUM(ImGui, WindowFlags_NoNavFocus,                "No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB).");
DEFINE_ENUM(ImGui, WindowFlags_UnsavedDocument,           "Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. When used in a tab/docking context, tab is selected on closure and closure is deferred by one frame to allow code to cancel the closure (with a confirmation popup, etc.) without flicker.");
DEFINE_ENUM(ImGui, WindowFlags_NoDocking,                 "Disable docking of this window.");
DEFINE_ENUM(ImGui, WindowFlags_NoNav,                     "ImGui_WindowFlags_NoNavInputs | ImGui_WindowFlags_NoNavFocus");
DEFINE_ENUM(ImGui, WindowFlags_NoDecoration,              "ImGui_WindowFlags_NoTitleBar | ImGui_WindowFlags_NoResize | ImGui_WindowFlags_NoScrollbar | ImGui_WindowFlags_NoCollapse");
DEFINE_ENUM(ImGui, WindowFlags_NoInputs,                  "ImGui_WindowFlags_NoMouseInputs | ImGui_WindowFlags_NoNavInputs | ImGui_WindowFlags_NoNavFocus");

// ReaImGui exclusive constants
DEFINE_ENUM(ReaImGui, ConfigFlags_NoSavedSettings, "Disable state restoration and persistence for the whole context.");
DEFINE_ENUM(ReaImGui, FontFlags_None, "");
DEFINE_ENUM(ReaImGui, FontFlags_Bold, "");
DEFINE_ENUM(ReaImGui, FontFlags_Italic, "");

DEFINE_API(void, NumericLimits_Float, (double*,API_W(min))(double*,API_W(max)),
"Returns FLT_MIN and FLT_MAX for this system.",
{
  assertValid(API_W(min));
  assertValid(API_W(max));
  *API_W(min) = FLT_MIN;
  *API_W(max) = FLT_MAX;
});
