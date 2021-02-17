#include "api_helper.hpp"

#define DEFINE_ENUM(name, doc) \
  DEFINE_API(int, name, NO_ARGS, doc, { return ImGui##name; })

// ImGuiCol
// Enum: A color identifier for styling
DEFINE_ENUM(Col_Text,                  "");
DEFINE_ENUM(Col_TextDisabled,          "");
DEFINE_ENUM(Col_WindowBg,              "Background of normal windows");
DEFINE_ENUM(Col_ChildBg,               "Background of child windows");
DEFINE_ENUM(Col_PopupBg,               "Background of popups, menus, tooltips windows");
DEFINE_ENUM(Col_Border,                "");
DEFINE_ENUM(Col_BorderShadow,          "");
DEFINE_ENUM(Col_FrameBg,               "Background of checkbox, radio button, plot, slider, text input");
DEFINE_ENUM(Col_FrameBgHovered,        "");
DEFINE_ENUM(Col_FrameBgActive,         "");
DEFINE_ENUM(Col_TitleBg,               "");
DEFINE_ENUM(Col_TitleBgActive,         "");
DEFINE_ENUM(Col_TitleBgCollapsed,      "");
DEFINE_ENUM(Col_MenuBarBg,             "");
DEFINE_ENUM(Col_ScrollbarBg,           "");
DEFINE_ENUM(Col_ScrollbarGrab,         "");
DEFINE_ENUM(Col_ScrollbarGrabHovered,  "");
DEFINE_ENUM(Col_ScrollbarGrabActive,   "");
DEFINE_ENUM(Col_CheckMark,             "");
DEFINE_ENUM(Col_SliderGrab,            "");
DEFINE_ENUM(Col_SliderGrabActive,      "");
DEFINE_ENUM(Col_Button,                "");
DEFINE_ENUM(Col_ButtonHovered,         "");
DEFINE_ENUM(Col_ButtonActive,          "");
DEFINE_ENUM(Col_Header,                "Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem");
DEFINE_ENUM(Col_HeaderHovered,         "");
DEFINE_ENUM(Col_HeaderActive,          "");
DEFINE_ENUM(Col_Separator,             "");
DEFINE_ENUM(Col_SeparatorHovered,      "");
DEFINE_ENUM(Col_SeparatorActive,       "");
DEFINE_ENUM(Col_ResizeGrip,            "");
DEFINE_ENUM(Col_ResizeGripHovered,     "");
DEFINE_ENUM(Col_ResizeGripActive,      "");
DEFINE_ENUM(Col_Tab,                   "");
DEFINE_ENUM(Col_TabHovered,            "");
DEFINE_ENUM(Col_TabActive,             "");
DEFINE_ENUM(Col_TabUnfocused,          "");
DEFINE_ENUM(Col_TabUnfocusedActive,    "");
DEFINE_ENUM(Col_PlotLines,             "");
DEFINE_ENUM(Col_PlotLinesHovered,      "");
DEFINE_ENUM(Col_PlotHistogram,         "");
DEFINE_ENUM(Col_PlotHistogramHovered,  "");
DEFINE_ENUM(Col_TableHeaderBg,         "Table header background");
DEFINE_ENUM(Col_TableBorderStrong,     "Table outer and header borders (prefer using Alpha=1.0 here)");
DEFINE_ENUM(Col_TableBorderLight,      "Table inner borders (prefer using Alpha=1.0 here)");
DEFINE_ENUM(Col_TableRowBg,            "Table row background (even rows)");
DEFINE_ENUM(Col_TableRowBgAlt,         "Table row background (odd rows)");
DEFINE_ENUM(Col_TextSelectedBg,        "");
DEFINE_ENUM(Col_DragDropTarget,        "");
DEFINE_ENUM(Col_NavHighlight,          "Gamepad/keyboard: current highlighted item");
DEFINE_ENUM(Col_NavWindowingHighlight, "Highlight window when using CTRL+TAB");
DEFINE_ENUM(Col_NavWindowingDimBg,     "Darken/colorize entire screen behind the CTRL+TAB window list, when active");
DEFINE_ENUM(Col_ModalWindowDimBg,      "Darken/colorize entire screen behind a modal window, when one is active");

// ImGuiCond
// Enumeration for ImGui::SetWindow***(), SetNextWindow***(), SetNextItem***() functions
// Represent a condition.
// Important: Treat as a regular enum! Do NOT combine multiple values using binary operators! All the functions above treat 0 as a shortcut to ImGuiCond_Always.
DEFINE_ENUM(Cond_Always, "No condition (always set the variable)");
DEFINE_ENUM(Cond_Once, "Set the variable once per runtime session (only the first call will succeed)");
DEFINE_ENUM(Cond_FirstUseEver, "Set the variable if the object/window has no persistently saved data (no entry in .ini file)");
DEFINE_ENUM(Cond_Appearing, "Set the variable if the object/window is appearing after being hidden/inactive (or the first time)");

// typedef int ImGuiDataType;          // -> enum ImGuiDataType_        // Enum: A primary data type

// ImGuiDir
DEFINE_ENUM(Dir_None,  "A cardinal direction");
DEFINE_ENUM(Dir_Left,  "A cardinal direction");
DEFINE_ENUM(Dir_Right, "A cardinal direction");
DEFINE_ENUM(Dir_Up,    "A cardinal direction");
DEFINE_ENUM(Dir_Down,  "A cardinal direction");

// typedef int ImGuiKey;               // -> enum ImGuiKey_             // Enum: A key identifier (ImGui-side enum)
// typedef int ImGuiNavInput;          // -> enum ImGuiNavInput_        // Enum: An input identifier for navigation
// ImGuiMouseButton
// Enum: A mouse button identifier (0=left, 1=right, 2=middle)
DEFINE_ENUM(MouseButton_Left, "");
DEFINE_ENUM(MouseButton_Right, "");
DEFINE_ENUM(MouseButton_Middle, "");

// typedef int ImGuiMouseCursor;       // -> enum ImGuiMouseCursor_     // Enum: A mouse cursor identifier
// typedef int ImGuiSortDirection;     // -> enum ImGuiSortDirection_   // Enum: A sorting direction (ascending or descending)

// ImGuiStyleVar
// Enum: A variable identifier for styling
DEFINE_ENUM(StyleVar_Alpha,               "Global alpha applies to everything in Dear ImGui.");
DEFINE_ENUM(StyleVar_WindowPadding,       "Padding within a window.");
DEFINE_ENUM(StyleVar_WindowRounding,      "Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.");
DEFINE_ENUM(StyleVar_WindowBorderSize,    "Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(StyleVar_WindowMinSize,       "Minimum window size. This is a global setting. If you want to constraint individual windows, use SetNextWindowSizeConstraints().");
DEFINE_ENUM(StyleVar_WindowTitleAlign,    "Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.");
DEFINE_ENUM(StyleVar_ChildRounding,       "Radius of child window corners rounding. Set to 0.0f to have rectangular windows.");
DEFINE_ENUM(StyleVar_ChildBorderSize,     "Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(StyleVar_PopupRounding,       "Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)");
DEFINE_ENUM(StyleVar_PopupBorderSize,     "Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(StyleVar_FramePadding,        "Padding within a framed rectangle (used by most widgets).");
DEFINE_ENUM(StyleVar_FrameRounding,       "Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).");
DEFINE_ENUM(StyleVar_FrameBorderSize,     "Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(StyleVar_ItemSpacing,         "Horizontal and vertical spacing between widgets/lines.");
DEFINE_ENUM(StyleVar_ItemInnerSpacing,    "Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).");
DEFINE_ENUM(StyleVar_IndentSpacing,       "Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).");
DEFINE_ENUM(StyleVar_CellPadding,         "Padding within a table cell");
DEFINE_ENUM(StyleVar_ScrollbarSize,       "Width of the vertical scrollbar, Height of the horizontal scrollbar.");
DEFINE_ENUM(StyleVar_ScrollbarRounding,   "Radius of grab corners for scrollbar.");
DEFINE_ENUM(StyleVar_GrabMinSize,         "Minimum width/height of a grab box for slider/scrollbar.");
DEFINE_ENUM(StyleVar_GrabRounding,        "Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.");
DEFINE_ENUM(StyleVar_TabRounding,         "Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.");
DEFINE_ENUM(StyleVar_ButtonTextAlign,     "Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).");
DEFINE_ENUM(StyleVar_SelectableTextAlign, "Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.");

// typedef int ImGuiTableBgTarget;     // -> enum ImGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()
// typedef int ImDrawCornerFlags;      // -> enum ImDrawCornerFlags_    // Flags: for ImDrawList::AddRect(), AddRectFilled() etc.
// typedef int ImDrawListFlags;        // -> enum ImDrawListFlags_      // Flags: for ImDrawList
// typedef int ImFontAtlasFlags;       // -> enum ImFontAtlasFlags_     // Flags: for ImFontAtlas build
// typedef int ImGuiBackendFlags;      // -> enum ImGuiBackendFlags_    // Flags: for io.BackendFlags
// typedef int ImGuiButtonFlags;       // -> enum ImGuiButtonFlags_     // Flags: for InvisibleButton()

// ImGuiColorEditFlags
// Flags for ColorEdit3() / ColorEdit4() / ColorPicker3() / ColorPicker4() / ColorButton()
DEFINE_ENUM(ColorEditFlags_None,             "");
DEFINE_ENUM(ColorEditFlags_NoAlpha,          "ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).");
DEFINE_ENUM(ColorEditFlags_NoPicker,         "ColorEdit: disable picker when clicking on color square.");
DEFINE_ENUM(ColorEditFlags_NoOptions,        "ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.");
DEFINE_ENUM(ColorEditFlags_NoSmallPreview,   "ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)");
DEFINE_ENUM(ColorEditFlags_NoInputs,         "ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).");
DEFINE_ENUM(ColorEditFlags_NoTooltip,        "ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.");
DEFINE_ENUM(ColorEditFlags_NoLabel,          "ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).");
DEFINE_ENUM(ColorEditFlags_NoSidePreview,    "ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.");
DEFINE_ENUM(ColorEditFlags_NoDragDrop,       "ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.");
DEFINE_ENUM(ColorEditFlags_NoBorder,         "ColorButton: disable border (which is enforced by default)");
// User Options (right-click on widget to change some of them).
DEFINE_ENUM(ColorEditFlags_AlphaBar,         "ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.");
DEFINE_ENUM(ColorEditFlags_AlphaPreview,     "ColorEdit, ColorPicker, ColorButton: display preview as a transparent color over a checkerboard, instead of opaque.");
DEFINE_ENUM(ColorEditFlags_AlphaPreviewHalf, "ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard, instead of opaque.");
// DEFINE_ENUM(ColorEditFlags_HDR,              "(WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use ImGuiColorEditFlags_Float flag as well).");
DEFINE_ENUM(ColorEditFlags_DisplayRGB,       "ColorEdit: override _display_ type to RGB. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ColorEditFlags_DisplayHSV,       "ColorEdit: override _display_ type to HSV. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ColorEditFlags_DisplayHex,       "ColorEdit: override _display_ type to Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ColorEditFlags_Uint8,            "ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.");
DEFINE_ENUM(ColorEditFlags_Float,            "ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.");
DEFINE_ENUM(ColorEditFlags_PickerHueBar,     "ColorPicker: bar for Hue, rectangle for Sat/Value.");
DEFINE_ENUM(ColorEditFlags_PickerHueWheel,   "ColorPicker: wheel for Hue, triangle for Sat/Value.");
DEFINE_ENUM(ColorEditFlags_InputRGB,         "ColorEdit, ColorPicker: input and output data in RGB format.");
DEFINE_ENUM(ColorEditFlags_InputHSV,         "ColorEdit, ColorPicker: input and output data in HSV format.");

// typedef int ImGuiConfigFlags;       // -> enum ImGuiConfigFlags_     // Flags: for io.ConfigFlags

// ImGuiComboFlags
// Flags for ImGui::BeginCombo()
DEFINE_ENUM(ComboFlags_None,           "");
DEFINE_ENUM(ComboFlags_PopupAlignLeft, "Align the popup toward the left by default");
DEFINE_ENUM(ComboFlags_HeightSmall,    "Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()");
DEFINE_ENUM(ComboFlags_HeightRegular,  "Max ~8 items visible (default)");
DEFINE_ENUM(ComboFlags_HeightLarge,    "Max ~20 items visible");
DEFINE_ENUM(ComboFlags_HeightLargest,  "As many fitting items as possible");
DEFINE_ENUM(ComboFlags_NoArrowButton,  "Display on the preview box without the square arrow button");
DEFINE_ENUM(ComboFlags_NoPreview,      "Display only a square arrow button");

// typedef int ImGuiDragDropFlags;     // -> enum ImGuiDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
// typedef int ImGuiFocusedFlags;      // -> enum ImGuiFocusedFlags_    // Flags: for IsWindowFocused()
// typedef int ImGuiHoveredFlags;      // -> enum ImGuiHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.

// ImGuiInputTextFlags
// Most of the ImGuiInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX, InputDouble etc.
DEFINE_ENUM(InputTextFlags_None,                "");
DEFINE_ENUM(InputTextFlags_CharsDecimal,        "Allow 0123456789.+-*/");
DEFINE_ENUM(InputTextFlags_CharsHexadecimal,    "Allow 0123456789ABCDEFabcdef");
DEFINE_ENUM(InputTextFlags_CharsUppercase,      "Turn a..z into A..Z");
DEFINE_ENUM(InputTextFlags_CharsNoBlank,        "Filter out spaces, tabs");
DEFINE_ENUM(InputTextFlags_AutoSelectAll,       "Select entire text when first taking mouse focus");
DEFINE_ENUM(InputTextFlags_EnterReturnsTrue,    "Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit() function.");
// DEFINE_ENUM(InputTextFlags_CallbackCompletion,  "Callback on pressing TAB (for completion handling)");
// DEFINE_ENUM(InputTextFlags_CallbackHistory,     "Callback on pressing Up/Down arrows (for history handling)");
// DEFINE_ENUM(InputTextFlags_CallbackAlways,      "Callback on each iteration. User code may query cursor position, modify text buffer.");
// DEFINE_ENUM(InputTextFlags_CallbackCharFilter,  "Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.");
DEFINE_ENUM(InputTextFlags_AllowTabInput,       "Pressing TAB input a '\\t' character into the text field");
DEFINE_ENUM(InputTextFlags_CtrlEnterForNewLine, "In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).");
DEFINE_ENUM(InputTextFlags_NoHorizontalScroll,  "Disable following the cursor horizontally");
DEFINE_ENUM(InputTextFlags_AlwaysInsertMode,    "Insert mode");
DEFINE_ENUM(InputTextFlags_ReadOnly,            "Read-only mode");
DEFINE_ENUM(InputTextFlags_Password,            "Password mode, display all characters as '*'");
DEFINE_ENUM(InputTextFlags_NoUndoRedo,          "Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().");
DEFINE_ENUM(InputTextFlags_CharsScientific,     "Allow 0123456789.+-*/eE (Scientific notation input)");
// DEFINE_ENUM(InputTextFlags_CallbackResize,      "Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this)");
// DEFINE_ENUM(InputTextFlags_CallbackEdit,        "Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)");
// [Internal]
// DEFINE_ENUM(InputTextFlags_Multiline,           "For internal use by InputTextMultiline()");
// DEFINE_ENUM(InputTextFlags_NoMarkEdited,        "For internal use by functions using InputText() before reformatting data");

// typedef int ImGuiKeyModFlags;       // -> enum ImGuiKeyModFlags_     // Flags: for io.KeyMods (Ctrl/Shift/Alt/Super)
// typedef int ImGuiPopupFlags;        // -> enum ImGuiPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()

// ImGuiSelectableFlags
// Flags: for Selectable()
DEFINE_ENUM(SelectableFlags_None,             "");
DEFINE_ENUM(SelectableFlags_DontClosePopups,  "Clicking this don't close parent popup window");
DEFINE_ENUM(SelectableFlags_SpanAllColumns,   "Selectable frame can span all columns (text will still fit in current column)");
DEFINE_ENUM(SelectableFlags_AllowDoubleClick, "Generate press events on double clicks too");
DEFINE_ENUM(SelectableFlags_Disabled,         "Cannot be selected, display grayed out text");
// DEFINE_ENUM(SelectableFlags_AllowItemOverlap, "(WIP) Hit testing to allow subsequent widgets to overlap this one");

// ImGuiSliderFlags
// for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
DEFINE_ENUM(SliderFlags_None,            "");
DEFINE_ENUM(SliderFlags_AlwaysClamp,     "Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.");
DEFINE_ENUM(SliderFlags_Logarithmic,     "Make the widget logarithmic (linear otherwise). Consider using ImGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.");
DEFINE_ENUM(SliderFlags_NoRoundToFormat, "Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits)");
DEFINE_ENUM(SliderFlags_NoInput,         "Disable CTRL+Click or Enter key allowing to input text directly into the widget");

// ImGuiTabBarFlags
// Flags: for BeginTabBar()
DEFINE_ENUM(TabBarFlags_None,                         "");
DEFINE_ENUM(TabBarFlags_Reorderable,                  "Allow manually dragging tabs to re-order them + New tabs are appended at the end of list");
DEFINE_ENUM(TabBarFlags_AutoSelectNewTabs,            "Automatically select new tabs when they appear");
DEFINE_ENUM(TabBarFlags_TabListPopupButton,           "Disable buttons to open the tab list popup");
DEFINE_ENUM(TabBarFlags_NoCloseWithMiddleMouseButton, "Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.");
DEFINE_ENUM(TabBarFlags_NoTabListScrollingButtons,    "Disable scrolling buttons (apply when fitting policy is ImGuiTabBarFlags_FittingPolicyScroll)");
DEFINE_ENUM(TabBarFlags_NoTooltip,                    "Disable tooltips when hovering a tab");
DEFINE_ENUM(TabBarFlags_FittingPolicyResizeDown,      "Resize tabs when they don't fit");
DEFINE_ENUM(TabBarFlags_FittingPolicyScroll,          "Add scroll buttons when tabs don't fit");

// ImGuiTabItemFlags
// Flags: for BeginTabItem()
DEFINE_ENUM(TabItemFlags_None,                         "");
DEFINE_ENUM(TabItemFlags_UnsavedDocument,              "Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. Also: tab is selected on closure and closure is deferred by one frame to allow code to undo it without flicker.");
DEFINE_ENUM(TabItemFlags_SetSelected,                  "Trigger flag to programmatically make the tab selected when calling BeginTabItem()");
DEFINE_ENUM(TabItemFlags_NoCloseWithMiddleMouseButton, "Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.");
DEFINE_ENUM(TabItemFlags_NoPushId,                     "Don't call PushID(tab->ID)/PopID() on BeginTabItem()/EndTabItem()");
DEFINE_ENUM(TabItemFlags_NoTooltip,                    "Disable tooltip for the given tab");
DEFINE_ENUM(TabItemFlags_NoReorder,                    "Disable reordering this tab or having another tab cross over this tab");
DEFINE_ENUM(TabItemFlags_Leading,                      "Enforce the tab position to the left of the tab bar (after the tab list popup button)");
DEFINE_ENUM(TabItemFlags_Trailing,                     "Enforce the tab position to the right of the tab bar (before the scrolling buttons)");

// ImGuiTableFlags
// for ImGui::BeginTable()
// - Important! Sizing policies have complex and subtle side effects, more so than you would expect.
//   Read comments/demos carefully + experiment with live demos to get acquainted with them.
// - The DEFAULT sizing policies are:
//    - Default to ImGuiTableFlags_SizingFixedFit    if ScrollX is on, or if host window has ImGuiWindowFlags_AlwaysAutoResize.
//    - Default to ImGuiTableFlags_SizingStretchSame if ScrollX is off.
// - When ScrollX is off:
//    - Table defaults to ImGuiTableFlags_SizingStretchSame -> all Columns defaults to ImGuiTableColumnFlags_WidthStretch with same weight.
//    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
//    - Fixed Columns will generally obtain their requested width (unless the table cannot fit them all).
//    - Stretch Columns will share the remaining width.
//    - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
//      The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
//      (this is because the visible order of columns have subtle but necessary effects on how they react to manual resizing).
// - When ScrollX is on:
//    - Table defaults to ImGuiTableFlags_SizingFixedFit -> all Columns defaults to ImGuiTableColumnFlags_WidthFixed
//    - Columns sizing policy allowed: Fixed/Auto mostly.
//    - Fixed Columns can be enlarged as needed. Table will show an horizontal scrollbar if needed.
//    - When using auto-resizing (non-resizable) fixed columns, querying the content width to use item right-alignment e.g. SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a feedback loop.
//    - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS you have specified a value for 'inner_width' in BeginTable().
//      If you specify a value for 'inner_width' then effectively the scrolling space is known and Stretch or mixed Fixed/Stretch columns become meaningful again.
// - Read on documentation at the top of imgui_tables.cpp for details.
// Features
DEFINE_ENUM(TableFlags_None,                       "");
DEFINE_ENUM(TableFlags_Resizable,                  "Enable resizing columns.");
DEFINE_ENUM(TableFlags_Reorderable,                "Enable reordering columns in header row (need calling TableSetupColumn() + TableHeadersRow() to display headers)");
DEFINE_ENUM(TableFlags_Hideable,                   "Enable hiding/disabling columns in context menu.");
DEFINE_ENUM(TableFlags_Sortable,                   "Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see ImGuiTableFlags_SortMulti and ImGuiTableFlags_SortTristate.");
// DEFINE_ENUM(TableFlags_NoSavedSettings,            "Disable persisting columns order, width and sort settings in the .ini file.");
DEFINE_ENUM(TableFlags_ContextMenuInBody,          "Right-click on columns body/contents will display table context menu. By default it is available in TableHeadersRow().");
// Decorations
DEFINE_ENUM(TableFlags_RowBg,                      "Set each RowBg color with ImGuiCol_TableRowBg or ImGuiCol_TableRowBgAlt (equivalent of calling TableSetBgColor with ImGuiTableBgFlags_RowBg0 on each row manually)");
DEFINE_ENUM(TableFlags_BordersInnerH,              "Draw horizontal borders between rows.");
DEFINE_ENUM(TableFlags_BordersOuterH,              "Draw horizontal borders at the top and bottom.");
DEFINE_ENUM(TableFlags_BordersInnerV,              "Draw vertical borders between columns.");
DEFINE_ENUM(TableFlags_BordersOuterV,              "Draw vertical borders on the left and right sides.");
DEFINE_ENUM(TableFlags_BordersH,                   "Draw horizontal borders.");
DEFINE_ENUM(TableFlags_BordersV,                   "Draw vertical borders.");
DEFINE_ENUM(TableFlags_BordersInner,               "Draw inner borders.");
DEFINE_ENUM(TableFlags_BordersOuter,               "Draw outer borders.");
DEFINE_ENUM(TableFlags_Borders,                    "Draw all borders.");
// DEFINE_ENUM(TableFlags_NoBordersInBody,            "[ALPHA] Disable vertical borders in columns Body (borders will always appears in Headers). -> May move to style");
// DEFINE_ENUM(TableFlags_NoBordersInBodyUntilResize, "[ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appears in Headers). -> May move to style");
// Sizing Policy (read above for defaults)
DEFINE_ENUM(TableFlags_SizingFixedFit,             "Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.");
DEFINE_ENUM(TableFlags_SizingFixedSame,            "Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable ImGuiTableFlags_NoKeepColumnsVisible.");
DEFINE_ENUM(TableFlags_SizingStretchProp,          "Columns default to _WidthStretch with default weights proportional to each columns contents widths.");
DEFINE_ENUM(TableFlags_SizingStretchSame,          "Columns default to _WidthStretch with default weights all equal, unless overriden by TableSetupColumn().");
// Sizing Extra Options
DEFINE_ENUM(TableFlags_NoHostExtendX,              "Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
DEFINE_ENUM(TableFlags_NoHostExtendY,              "Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
DEFINE_ENUM(TableFlags_NoKeepColumnsVisible,       "Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.");
DEFINE_ENUM(TableFlags_PreciseWidths,              "Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
// Clipping
DEFINE_ENUM(TableFlags_NoClip,                     "Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().");
// Padding
DEFINE_ENUM(TableFlags_PadOuterX,                  "Default if BordersOuterV is on. Enable outer-most padding. Generally desirable if you have headers.");
DEFINE_ENUM(TableFlags_NoPadOuterX,                "Default if BordersOuterV is off. Disable outer-most padding.");
DEFINE_ENUM(TableFlags_NoPadInnerX,                "Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).");
// Scrolling
DEFINE_ENUM(TableFlags_ScrollX,                    "Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this create a child window, ScrollY is currently generally recommended when using ScrollX.");
DEFINE_ENUM(TableFlags_ScrollY,                    "Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.");
// Sorting
DEFINE_ENUM(TableFlags_SortMulti,                  "Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
DEFINE_ENUM(TableFlags_SortTristate,               "Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");

// typedef int ImGuiTableColumnFlags;  // -> enum ImGuiTableColumnFlags_// Flags: For TableSetupColumn()
// typedef int ImGuiTableRowFlags;     // -> enum ImGuiTableRowFlags_   // Flags: For TableNextRow()

// ImGuiTreeNodeFlags
// Flags for TreeNode(), TreeNodeEx(), CollapsingHeader()
DEFINE_ENUM(TreeNodeFlags_None,                 "");
DEFINE_ENUM(TreeNodeFlags_Selected,             "Draw as selected");
DEFINE_ENUM(TreeNodeFlags_Framed,               "Draw frame with background (e.g. for CollapsingHeader)");
DEFINE_ENUM(TreeNodeFlags_AllowItemOverlap,     "Hit testing to allow subsequent widgets to overlap this one");
DEFINE_ENUM(TreeNodeFlags_NoTreePushOnOpen,     "Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack");
DEFINE_ENUM(TreeNodeFlags_NoAutoOpenOnLog,      "Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)");
DEFINE_ENUM(TreeNodeFlags_DefaultOpen,          "Default node to be open");
DEFINE_ENUM(TreeNodeFlags_OpenOnDoubleClick,    "Need double-click to open node");
DEFINE_ENUM(TreeNodeFlags_OpenOnArrow,          "Only open when clicking on the arrow part. If ImGuiTreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.");
DEFINE_ENUM(TreeNodeFlags_Leaf,                 "No collapsing, no arrow (use as a convenience for leaf nodes).");
DEFINE_ENUM(TreeNodeFlags_Bullet,               "Display a bullet instead of arrow");
DEFINE_ENUM(TreeNodeFlags_FramePadding,         "Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().");
DEFINE_ENUM(TreeNodeFlags_SpanAvailWidth,       "Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.");
DEFINE_ENUM(TreeNodeFlags_SpanFullWidth,        "Extend hit box to the left-most and right-most edges (bypass the indented area).");
// DEFINE_ENUM(TreeNodeFlags_NavLeftJumpsBackHere, "(WIP) Nav: left direction may move to this TreeNode() from any of its child (items submitted between TreeNode and TreePop)");
// DEFINE_ENUM(TreeNodeFlags_NoScrollOnOpen        "FIXME: TODO: Disable automatic scroll on TreePop() if node got just open and contents is not visible")
// DEFINE_ENUM(ImGuiTreeNodeFlags_CollapsingHeader,"");

// ImGuiWindowFlags
// for Begin(), BeginChild()
DEFINE_ENUM(WindowFlags_None,                      "Default flag. See #ImGui_Begin.");
DEFINE_ENUM(WindowFlags_NoTitleBar,                "Disable title-bar");
DEFINE_ENUM(WindowFlags_NoResize,                  "Disable user resizing with the lower-right grip");
DEFINE_ENUM(WindowFlags_NoMove,                    "Disable user moving the window");
DEFINE_ENUM(WindowFlags_NoScrollbar,               "Disable scrollbars (window can still scroll with mouse or programmatically)");
DEFINE_ENUM(WindowFlags_NoScrollWithMouse,         "Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.");
DEFINE_ENUM(WindowFlags_NoCollapse,                "Disable user collapsing window by double-clicking on it");
DEFINE_ENUM(WindowFlags_AlwaysAutoResize,          "Resize every window to its content every frame");
DEFINE_ENUM(WindowFlags_NoBackground,              "Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).");
// DEFINE_ENUM(WindowFlags_NoSavedSettings,           "Never load/save settings in .ini file");
DEFINE_ENUM(WindowFlags_NoMouseInputs,             "Disable catching mouse, hovering test with pass through.");
DEFINE_ENUM(WindowFlags_MenuBar,                   "Has a menu-bar");
DEFINE_ENUM(WindowFlags_HorizontalScrollbar,     R"(Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.)");
DEFINE_ENUM(WindowFlags_NoFocusOnAppearing,        "Disable taking focus when transitioning from hidden to visible state");
DEFINE_ENUM(WindowFlags_NoBringToFrontOnFocus,     "Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)");
DEFINE_ENUM(WindowFlags_AlwaysVerticalScrollbar,   "Always show vertical scrollbar (even if ContentSize.y < Size.y)");
DEFINE_ENUM(WindowFlags_AlwaysHorizontalScrollbar, "Always show horizontal scrollbar (even if ContentSize.x < Size.x)");
DEFINE_ENUM(WindowFlags_AlwaysUseWindowPadding,    "Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)");
DEFINE_ENUM(WindowFlags_NoNavInputs,               "No gamepad/keyboard navigation within the window");
DEFINE_ENUM(WindowFlags_NoNavFocus,                "No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)");
DEFINE_ENUM(WindowFlags_UnsavedDocument,           "Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. When used in a tab/docking context, tab is selected on closure and closure is deferred by one frame to allow code to cancel the closure (with a confirmation popup, etc.) without flicker.");
DEFINE_ENUM(WindowFlags_NoNav,                     "WindowFlags_NoNavInputs | WindowFlags_NoNavFocus");
DEFINE_ENUM(WindowFlags_NoDecoration,              "WindowFlags_NoTitleBar | WindowFlags_NoResize | WindowFlags_NoScrollbar | WindowFlags_NoCollapse");
DEFINE_ENUM(WindowFlags_NoInputs,                  "WindowFlags_NoMouseInputs | WindowFlags_NoNavInputs | WindowFlags_NoNavFocus");
