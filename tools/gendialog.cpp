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

#include "../src/dialog.hpp"

#include <iostream>
#include <variant>
#include <version.hpp>
#include <vector>

// Using this instead of hand-written .rc files for:
// - Avoid PHP dependency on non-Windows (swell-resgen.php)
// - Consistent layout (no magic numbers for each x/y/w/h)
//
// Recommended sizing and spacing
// https://learn.microsoft.com/en-us/windows/win32/uxguide/vis-layout#recommended-sizing-and-spacing
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bb226818(v=vs.85)

#ifdef _WIN32
#  include <windows.h>
// avoid bogus "not all control paths return a value" warning with enum class
#  define UNREACHABLE __assume(false)
constexpr DWORD DIALOG_STYLE
  {DS_MODALFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION};
#else
#  include <swell/swell.h>
#  include <swell/swell-dlggen.h>
#  define UNREACHABLE __builtin_unreachable()
constexpr DWORD DIALOG_STYLE {};
constexpr DWORD DS_CONTROL {}, WS_EX_CONTROLPARENT {};
#endif

class OutputRC;
static OutputRC *g_rc;

class OutputRC {
public:
  OutputRC(std::ostream &stream);
  OutputRC(const OutputRC &) = delete;
  ~OutputRC() { g_rc = m_prev; }

  std::ostream &out;

private:
  OutputRC *m_prev;
};

// Utility types
enum class Axis { Vertical, Horizontal };
enum class Align { Start, Middle, End };

Axis operator!(const Axis axis)
{
  switch(axis) {
  case Axis::Vertical:
    return Axis::Horizontal;
  case Axis::Horizontal:
    return Axis::Vertical;
  }
  UNREACHABLE;
}

struct Box {
  int &pos(Axis);
  int &size(Axis);
  const int &pos(Axis a)  const { return const_cast<Box *>(this)->pos(a);  }
  const int &size(Axis a) const { return const_cast<Box *>(this)->size(a); }

  void addPadding(int);
  void addPadding(Axis, int);

  int x, y, w, h;
};

std::ostream &operator<<(std::ostream &stream, const Box &b)
{
  g_rc->out << b.x << ", " << b.y << ", " << b.w << ", " << b.h;
  return stream;
}

struct Dimension {
  static constexpr int DEF_MARGIN {4};

  Dimension() : base {}, margin {DEF_MARGIN}, canStretch {} {}
  Dimension(int base) : Dimension {base, DEF_MARGIN} {}
  Dimension(int base, int margin)
    : base {base }, margin {margin}, canStretch {base < 1} {};
  Dimension(int base, int margin, bool canStretch)
    : base {base }, margin {margin}, canStretch {canStretch} {}

  int base, margin;
  bool canStretch;
};

struct Size {
  Size() : width {}, height {} {}
  Size(int w, int h) : width {w}, height {h} {}
  Size(Dimension w, Dimension h) : width {w}, height {h} {}

  void merge(const Size &, Axis, bool includeMargin);
  Dimension &operator[](Axis);
  const Dimension &operator[](const Axis axis) const {
    return (*const_cast<Size *>(this))[axis];
  }

  Dimension width, height;
};

class StringLiteral {
public:
  StringLiteral(const char *str) : m_str {str} {}
  std::ostream &operator()(std::ostream &) const;

private:
  const char *m_str;
};

std::ostream &operator<<(std::ostream &stream, const StringLiteral &lit)
{
  return lit(stream);
}

std::ostream &StringLiteral::operator()(std::ostream &stream) const
{
  stream << '"';
  for(const char *p {m_str}; *p; ++p) {
    switch(*p) {
    case '\\':
      stream << "\\\\";
      break;
    case '\n':
      stream << "\\n";
      break;
    case '"':
      stream << "\\\"";
      break;
    default:
      stream << *p;
    }
  }
  stream << '"';
  return stream;
}

int &Box::pos(const Axis axis)
{
  switch(axis) {
  case Axis::Vertical:
    return y;
  case Axis::Horizontal:
    return x;
  }
  UNREACHABLE;
}

int &Box::size(const Axis axis)
{
  switch(axis) {
  case Axis::Vertical:
    return h;
  case Axis::Horizontal:
    return w;
  }
  UNREACHABLE;
}

void Box::addPadding(const int padding)
{
  addPadding(Axis::Vertical,   padding);
  addPadding(Axis::Horizontal, padding);
}

void Box::addPadding(const Axis axis, const int padding)
{
  pos(axis) += padding;
  size(axis) -= padding * 2;
}

Dimension &Size::operator[](const Axis axis)
{
  switch(axis) {
  case Axis::Vertical:
    return height;
  case Axis::Horizontal:
    return width;
  }
  UNREACHABLE;
}

void Size::merge(const Size &o, const Axis axis, const bool includeMargin)
{
  (*this)[axis].base += o[axis].base + (includeMargin * o[axis].margin);
  (*this)[!axis].base = std::max((*this)[!axis].base, o[!axis].base);
  width.canStretch  |= o.width.canStretch;
  height.canStretch |= o.height.canStretch;
}

// Widget types
class Button;
class CheckBox;
class ComboBox;
class Dummy;
class EditText;
class GroupBox;
class Indent;
class Layout;
class Spacing;
class Text;

using Widget = std::variant<
  Dummy, Indent, Layout, Spacing,
  Button, CheckBox, ComboBox, EditText, GroupBox, Text
>;

// Does not extend lifetime! Only valid because objects within are exclusively
// used from within Dialog's constructor. This includes the references held
// by layout objects.
using WidgetList = std::initializer_list<Widget>;

class WidgetVisitor {
public:
  WidgetVisitor(const Widget &widget) : m_widget {widget} {}

  Size size() const;
  void output(const Box &box) const;

private:
  const Widget &m_widget;
};

class NamedWidget {
public:
  NamedWidget(int id, const char *label, Size size)
    : m_id {id}, m_label {label}, m_size {size} {}
  Size size() const { return m_size; }

protected:
  int m_id;
  const char *m_label;
  Size m_size;
};

class Button : public NamedWidget {
public:
  Button(int id, const char *label, int w = 16, bool isDefault = false);
  void output(const Box &) const;

private:
  bool m_default;
};

class CheckBox : public NamedWidget {
public:
  CheckBox(int id, const char *label);
  void output(const Box &) const;
};

class ComboBox {
public:
  ComboBox(int id, int w);
  Size size() const { return m_size; }
  void output(const Box &) const;

private:
  int m_id;
  Size m_size;
};

class Dummy {
public:
  Dummy(int w, int h) : m_size {{w, 0}, {h, 0}} {} // no margin
  Size size() const { return m_size; }
  void output(const Box &) const {}

private:
  Size m_size;
};

class EditText {
public:
  EditText(int id, int width, int height, DWORD style);

  Size size() const { return m_size; }
  void output(const Box &) const;

private:
  int m_id;
  DWORD m_style;
  Size m_size;
};

class GroupBox : public NamedWidget {
public:
  GroupBox(int id, const char *label, const Widget &);
  void output(const Box &) const;

private:
  const Widget &m_widget;
};

class Indent {
public:
  // explicit to solve "recursive template instantiation exceeded maximum depth
  // of 1024" error with boost::variant2
  explicit Indent(const Widget &);
  Size size() const { return m_size; }
  void output(const Box &) const;

private:
  Size m_size;
  const Widget &m_widget;
};

class Layout {
public:
  Layout(Axis, Align align, WidgetList);

  Size size() const { return m_size; }
  void output(const Box &) const;

private:
  std::vector<Box> layout(const Box &) const;

  Axis  m_axis;
  Align m_align;
  Size  m_size;
  WidgetList m_widgets;
};

template<Axis axis>
struct MakeLayout : Layout {
  MakeLayout(WidgetList w) : Layout {axis, Align::Start, w} {}
  MakeLayout(Align align, WidgetList w) : Layout {axis, align, w} {}
};

using HLayout = MakeLayout<Axis::Horizontal>;
using VLayout = MakeLayout<Axis::Vertical>;

class Spacing : public Dummy {
public:
  Spacing() : Dummy(4, 4) {}
  void output(const Box &) const;
};

class Text : public NamedWidget {
public:
  Text(int id, const char *label, int width = 0);
  void output(const Box &) const;
};

Size WidgetVisitor::size() const
{
  return std::visit([](const auto &w) { return w.size(); }, m_widget);
}

void WidgetVisitor::output(const Box &box) const
{
  return std::visit([&box](const auto &w) { w.output(box); }, m_widget);
}

Button::Button(const int id, const char *label, const int w, const bool isDefault)
  : NamedWidget {id, label, {w, 14}}, m_default {isDefault}
{
}

void Button::output(const Box &box) const
{
  g_rc->out << "  " << (m_default ? "DEFPUSHBUTTON " : "PUSHBUTTON ")
            << StringLiteral(m_label) << ", " << m_id << ", " << box << '\n';
}

#ifdef _WIN32
constexpr int comboBoxHeight {14};
#else
constexpr int comboBoxHeight {12}; // special case for SWELL
#endif
ComboBox::ComboBox(const int id, const int w)
  : m_id {id}, m_size {w, comboBoxHeight}
{
}

void ComboBox::output(const Box &box) const
{
  g_rc->out << "  COMBOBOX " << m_id << ", " << box << ", "
            << (CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP) << '\n';
}

CheckBox::CheckBox(const int id, const char *label)
  : NamedWidget {id, label, {{0, 2}, {10, 2}}}
{
}

void CheckBox::output(const Box &box) const
{
  g_rc->out << "  CHECKBOX " << StringLiteral(m_label) << ", " << m_id << ", "
            << box << ", " << (BS_AUTOCHECKBOX | WS_TABSTOP) << '\n';
}

EditText::EditText(const int id, const int w, const int h, const DWORD style)
  : m_id {id}, m_style {style}, m_size {w, h}
{
}

void EditText::output(const Box &box) const
{
  g_rc->out << "  EDITTEXT " << m_id << ", " << box << ", " << m_style << '\n';
}

GroupBox::GroupBox(const int id, const char *label, const Widget &widget)
  : NamedWidget {id, label, {}}, m_widget {widget}
{
}

void GroupBox::output(const Box &box) const
{
  g_rc->out << "  GROUPBOX " << StringLiteral(m_label) << ", " << m_id << ", "
            << box << '\n';

  constexpr int PAD {7}, FROM_TOP {11 - PAD};
  Box contentBox {box};
  contentBox.addPadding(7);
  contentBox.y += FROM_TOP, contentBox.h -= FROM_TOP;
  WidgetVisitor {m_widget}.output(contentBox);
}

Indent::Indent(const Widget &widget)
  : m_size {WidgetVisitor {widget}.size()}, m_widget {widget}
{
}

void Indent::output(const Box &box) const
{
  Box contentBox {box};
  contentBox.x += 12;
  contentBox.w -= 12;

  g_rc->out << "  // -->\n";
  WidgetVisitor {m_widget}.output(contentBox);
  g_rc->out << "  // <--\n";
}

Layout::Layout(const Axis axis, const Align align, WidgetList widgets)
  : m_axis {axis}, m_align {align}, m_size {}, m_widgets(widgets)
{
  for(const Widget &widget : widgets) {
    const Size size {WidgetVisitor {widget}.size() };
    m_size.merge(size, m_axis, &widget != &*std::rbegin(widgets));
  }

  if(m_align != Align::Start)
    m_size[m_axis].canStretch = true;
}

std::vector<Box> Layout::layout(const Box &parentBox) const
{
  int x {parentBox.x}, y {parentBox.y};
  std::vector<Box> boxes;
  boxes.reserve(m_widgets.size());

  int *pos;
  switch(m_axis) {
  case Axis::Vertical:
    pos = &y;
    break;
  case Axis::Horizontal:
    pos = &x;
    break;
  }

  // populate boxes with base positions and count stretchable widgets
  int canStretchCount {};
  for(const Widget &widget : m_widgets) {
    Size size {WidgetVisitor {widget}.size() };
    int offAxisRemaining {parentBox.size(!m_axis) - size[!m_axis].base};
    if(offAxisRemaining > 0) {
      if(size[!m_axis].canStretch) {
        size[!m_axis].base = parentBox.size(!m_axis);
        offAxisRemaining = 0;
      }
      else if(m_align == Align::Start) // TODO: different on/off-axis alignment?
        offAxisRemaining = 0;
      else if(m_align == Align::Middle)
        offAxisRemaining /= 2;
    }
    boxes.push_back({x, y, size.width.base, size.height.base});
    if(offAxisRemaining > 0)
      boxes.back().pos(!m_axis) += offAxisRemaining;

    *pos += size[m_axis].base;
    if(&widget != &*std::rbegin(m_widgets))
      *pos += size[m_axis].margin;

    canStretchCount += size[m_axis].canStretch;
  }

  // adjust positions for alignment and stretching
  const int used {*pos - parentBox.pos(m_axis)};
  int remaining {parentBox.size(m_axis) - used};
  if(remaining > 0) {
    if(canStretchCount > 0) {
      const int stretchSize {remaining / canStretchCount};
      int stretchedCount {};
      auto widgetBox {boxes.begin()};
      for(const Widget &widget : m_widgets) {
        const Size size {WidgetVisitor {widget}.size()};
        widgetBox->pos(m_axis) += stretchSize * stretchedCount;
        if(size[m_axis].canStretch) {
          widgetBox->size(m_axis) += stretchSize;
          ++stretchedCount;
        }
        widgetBox++;
      }
    }
    else if(m_align != Align::Start) {
      if(m_align == Align::Middle)
        remaining /= 2;
      for(Box &box : boxes)
        box.pos(m_axis) += remaining;
    }
  }

  return boxes;
}

void Layout::output(const Box &box) const
{
  const std::vector<Box> &boxes {layout(box)};
  auto widgetBox {boxes.begin()};
  for(const Widget &widget : m_widgets)
    WidgetVisitor {widget}.output(*widgetBox++);
}

void Spacing::output(const Box &) const
{
  g_rc->out << '\n';
}

static int countLines(const char *label)
{
  int count {1};
  while(*label) {
    if(*label++ == '\n')
      ++count;
  }
  return count;
}

// recommended spacing between paragraphs is 8 but that's a bit too much
Text::Text(const int id, const char *label, int width)
  : NamedWidget {id, label, {width, 8 * countLines(label)}}
{
}

void Text::output(const Box &box) const
{
  g_rc->out << "  LTEXT " << StringLiteral(m_label) << ", " << m_id << ", "
            << box << '\n';
}

class Dialog {
public:
  Dialog(int id, const char *title, int x, int y, int w, int h,
    DWORD style, DWORD exstyle, Widget);

private:
  Box m_box;
};

Dialog::Dialog(int id, const char *title, int x, int y, int w, int h,
    DWORD style, DWORD exstyle, Widget widget)
  : m_box {0, 0, w, h}
{
  g_rc->out << '\n';

#ifdef _WIN32
  g_rc->out << id << " DIALOGEX "
            << x << ", " << y << ", " << w << ", " << h << '\n'
            << "STYLE   " << style   << '\n'
            << "EXSTYLE " << exstyle << '\n'
            << "FONT    8, " << StringLiteral("MS Shell Dlg") << '\n';
#else
  int flags {SWELL_DLG_WS_FLIPPED | SWELL_DLG_WS_NOAUTOSIZE};
  if(style & WS_CHILD)
    flags |= SWELL_DLG_WS_CHILD;

#  ifdef __APPLE__
  constexpr double dluScale {1.7};
#  else
  constexpr double dluScale {1.9};
#  endif

  g_rc->out << "SWELL_DEFINE_DIALOG_RESOURCE_BEGIN("
            << id << ", " << flags << ", " << StringLiteral(title) << ", "
            << w << ", " << h << ", " << dluScale << ")\n";
#endif

  if(!(style & WS_CHILD))
    m_box.addPadding(7);

  g_rc->out << "BEGIN\n";
  WidgetVisitor {widget}.output(m_box);
  g_rc->out << "END\n";

#ifndef _WIN32
  g_rc->out << "SWELL_DEFINE_DIALOG_RESOURCE_END(" << id << ")\n";
#endif
}

OutputRC::OutputRC(std::ostream &stream)
  : out {stream}, m_prev {g_rc}
{
  g_rc = this;
#ifdef _WIN32
  out << "#pragma code_page(" << CP_UTF8 << ")\n"
      << "#include <winres.h>\n";
#else
  out << "#include <swell/swell-dlggen.h>\n";
#endif
}

int main()
{
  OutputRC rc {std::cout};

  Dialog {IDD_SETTINGS, "", 0, 0, 319, 240,
          DS_CONTROL | WS_CHILD, WS_EX_CONTROLPARENT,
    GroupBox {IDC_GROUPBOX, "ReaImGui settings", VLayout {
      CheckBox {IDC_SAVEDSETTINGS, ""},
      Spacing {},
      CheckBox {IDC_DOCKINGENABLE, ""},
      Indent {VLayout {
        CheckBox {IDC_DOCKWITHSHIFT,   ""},
        CheckBox {IDC_DOCKSPLIT,       ""},
        CheckBox {IDC_DOCKTRANSPARENT, ""},
      }},
      Spacing {},

      HLayout {Align::Middle, {
        Text {IDC_RENDERERTXT, "", 98},
        ComboBox {IDC_RENDERER, 72},
        CheckBox {IDC_FORCESOFTWARE, ""},
      }},
      Spacing {},

      Dummy {0, 0},

      HLayout {Align::End, {
        Text {IDC_VERSION, "v" REAIMGUI_VERSION}, // no WS_DISABLED for SWELL
        Button {IDC_RESETDEFAULTS, "Restore defaults", 70},
      }},
    }},
  };

  Dialog {IDD_ERROR, "", 50, 50, 320, 114, DIALOG_STYLE, 0, VLayout {
    Dummy {0, 12},
    Text  {IDC_MESSAGE, "\n\n"},
    EditText {IDC_REPORT, 0, 0, WS_VSCROLL | ES_MULTILINE | ES_READONLY},
    HLayout {
      Button {IDC_SETTINGS, "ReaImGui &settings...", 80},
      HLayout {Align::End, {
        Button {IDC_PREV, "←"},
        Button {IDC_NEXT, "→"},
        Spacing {},
        Button {IDOK, "&Close", 50, true},
      }},
    },
  }};
}
