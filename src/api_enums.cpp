#include "api_helper.hpp"

#define DEFINE_ENUM(name, doc) \
  DEFINE_API(int, name, (()), doc, { return ImGui##name; })

// typedef int ImGuiCol;               // -> enum ImGuiCol_             // Enum: A color identifier for styling

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
// typedef int ImGuiStyleVar;          // -> enum ImGuiStyleVar_        // Enum: A variable identifier for styling
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
// typedef int ImGuiComboFlags;        // -> enum ImGuiComboFlags_      // Flags: for BeginCombo()
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
// typedef int ImGuiSelectableFlags;   // -> enum ImGuiSelectableFlags_ // Flags: for Selectable()

// ImGuiSliderFlags
// for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
DEFINE_ENUM(SliderFlags_None,            "");
DEFINE_ENUM(SliderFlags_AlwaysClamp,     "Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.");
DEFINE_ENUM(SliderFlags_Logarithmic,     "Make the widget logarithmic (linear otherwise). Consider using ImGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.");
DEFINE_ENUM(SliderFlags_NoRoundToFormat, "Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits)");
DEFINE_ENUM(SliderFlags_NoInput,         "Disable CTRL+Click or Enter key allowing to input text directly into the widget");

// typedef int ImGuiTabBarFlags;       // -> enum ImGuiTabBarFlags_     // Flags: for BeginTabBar()
// typedef int ImGuiTabItemFlags;      // -> enum ImGuiTabItemFlags_    // Flags: for BeginTabItem()
// typedef int ImGuiTableFlags;        // -> enum ImGuiTableFlags_      // Flags: For BeginTable()
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
