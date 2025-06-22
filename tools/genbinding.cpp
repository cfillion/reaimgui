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

#include <version.hpp>

#include "../src/api.hpp"
#include "../src/import.hpp"
#include "../src/win32_unicode.hpp"

#include <algorithm>
#include <boost/preprocessor/stringize.hpp>
#include <cassert>
#include <cfloat> // FLT_{MIN,MAX}
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <md4c-html.h>

static char API_VERSION[255];
constexpr const char *GENERATED_FOR
  {"Generated for ReaImGui v" REAIMGUI_VERSION};

struct Type {
  Type(const char *val)             : m_value {val} {}
  Type(const std::string_view &val) : m_value {val} {}

  bool isVoid()      const { return m_value == "void"; }
  bool isInt()       const { return m_value == "int"; }
  bool isBool()      const { return m_value == "bool"; }
  bool isDouble()    const { return m_value == "double"; }
  bool isString()    const { return m_value == "const char*" || m_value == "char*"; }
  bool isPointer()   const { return m_value.size() >= 1 && m_value.back() == '*'; }
  bool isConst()     const { return m_value.find("const ") == 0; }
  bool isScalar()    const { return isBool() || isInt() || isDouble(); }
  bool isScalarPtr() const { return isPointer() && removePtr().isScalar(); }
  bool isImVec2()    const { return m_value == "ImVec2"; }

  Type removePtr() const
  {
    if(isPointer())
      return {std::string_view {m_value.data(), m_value.size() - 1}};
    else
      return *this;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Type &t)
  {
    stream << t.m_value;
    return stream;
  }

  operator const std::string_view &() const { return m_value; }
  bool operator==(const std::string_view &o) const { return m_value == o; }

private:
  std::string_view m_value;
};

struct Argument {
  Type type;
  std::string_view name;
  std::string_view defv;

  bool isInput() const
    { return !isOutput() || name.find("In") != std::string_view::npos; }
  bool isOutput() const
    { return name.find("Out") != std::string_view::npos; }
  bool isOptional() const
    { return name.find("Optional") != std::string_view::npos; }
  bool isBigBuf() const
    { return name.find("NeedBig") != std::string_view::npos; }
  bool isBufSize() const
    { return name.size() > 3 && name.find("_sz") == name.size() - 3; }
  std::string_view bufName() const { return name.substr(0, name.size() - 3); }

  std::string_view humanName() const;
  void defaultValue(std::ostream &, const char *null) const;
};

struct Function {
  Function(const API::Symbol *);

  const API::Section *section;
  std::string_view name;
  Type type;
  API::LineNumber line;
  std::vector<Argument> args;
  std::string_view doc;
  std::deque<const API::Section *> sections;
  VerNum version;
  int flags;

  bool operator<(const Function &) const;

  bool isVar()  const { return flags & API::Symbol::Variable; }
  bool isEnum() const { return flags & API::Symbol::Constant; }
  bool isNoDiscard() const
  {
    return (type.isBool() && name.substr(0, 5) == "Begin") ||
      name.substr(0, 6) == "Create";
  }
  bool hasOutputArgs() const;
  bool hasOptionalArgs() const;

  void cppSignature(std::ostream &) const;
  void luaSignature(std::ostream &) const;
  void eelSignature(std::ostream &, bool legacySyntax = false) const;
  void pythonSignature(std::ostream &) const;
};

bool Function::operator<(const Function &o) const
{
  const size_t maxDepth {std::min(sections.size(), o.sections.size())};
  for(size_t i {}; i < maxDepth; ++i) {
    if(sections[i] != o.sections[i])
      return strcmp(sections[i]->title, o.sections[i]->title) < 0;
  }

  if(sections.size() != o.sections.size())
    return sections.size() < o.sections.size();

  return name < o.name;
}

static std::deque<Function> g_funcs;

static const char *nextString(const char *&str)
{
  return str += strlen(str) + 1;
}

static std::string_view defaultValue(const std::string_view value)
{
  if(value.empty())
    return value;

  struct Macro { std::string_view name, value; };
  constexpr Macro macros[] {
#define MACRO(m) {"-" #m, BOOST_PP_STRINGIZE(m)}
    MACRO(FLT_MIN), MACRO(FLT_MAX)
#undef MACRO
  };

  for(const Macro &macro : macros) {
    assert(macro.name != macro.value); // ensure the macro is defined
    const bool isNeg {value[0] == '-'};
    if(value.substr(isNeg) == macro.value)
      return macro.name.substr(!isNeg);
  }

  return value;
}

Function::Function(const API::Symbol *api)
  : section {api->m_section}, name {api->name()},
    type {api->definition()}, line {api->m_line},
    version {api->version()}, flags {api->m_flags}
{
  const API::Section *curSection {section};
  do { sections.push_front(curSection); }
  while((curSection = curSection->parent));

  const char *def {api->definition()};
  std::string_view argTypes {nextString(def)};
  std::string_view argNames {nextString(def)};
  doc = nextString(def);
  std::string_view argDefvs {nextString(def)}; // non-standard field

  while(argTypes.size() > 0 && argNames.size() > 0) { // argDefvs may be empty
    size_t typeLen {argTypes.find(',')},
           nameLen {argNames.find(',')},
           defvLen {argDefvs.find('\31')};

    if(argDefvs.substr(0, strlen("ImGui")) == "ImGui") {
      argDefvs.remove_prefix(strlen("ImGui"));
      defvLen -= strlen("ImGui");
    }
    else if(argDefvs.substr(0, strlen("ImDraw")) == "ImDraw") {
      argDefvs.remove_prefix(strlen("Im"));
      defvLen -= strlen("Im");
    }
    else if(argDefvs.substr(0, strlen("ReaImGui")) == "ReaImGui") {
      argDefvs.remove_prefix(strlen("ReaImGui"));
      defvLen -= strlen("ReaImGui");
    }

    args.emplace_back(Argument {
      argTypes.substr(0, typeLen),
      argNames.substr(0, nameLen),
      defaultValue(argDefvs.substr(0, defvLen)),
    });

    if(typeLen == std::string_view::npos ||
       nameLen == std::string_view::npos ||
       defvLen == std::string_view::npos)
      break;

    argTypes.remove_prefix(typeLen + 1);
    argNames.remove_prefix(nameLen + 1);
    argDefvs.remove_prefix(defvLen + 1);
  }
}

struct CommaSep {
  CommaSep(std::ostream &stream, const char *sep = ", ")
    : m_stream {stream}, m_sep {sep}, m_printSep {false} {}

  template<typename T>
  std::ostream &operator<<(const T rhs)
  {
    if(m_printSep)
      m_stream << m_sep;
    else
      m_printSep = true;

    m_stream << rhs;
    return m_stream;
  }

private:
  std::ostream &m_stream;
  const char *m_sep;
  bool m_printSep;
};

static void cppBinding(std::ostream &stream)
{
  stream << "// " << GENERATED_FOR << R"(

#ifndef REAPER_IMGUI_FUNCTIONS_H
#define REAPER_IMGUI_FUNCTIONS_H

#include <stdexcept>
#include <type_traits>
#include <utility>

class ImGui_Context;
class ImGui_DrawList;
class ImGui_DrawListSplitter;
class ImGui_Font;
class ImGui_Function;
class ImGui_Image;
class ImGui_ImageSet;
class ImGui_ListClipper;
class ImGui_Resource;
class ImGui_TextFilter;
class ImGui_Viewport;

struct ImGui_Error : std::runtime_error {
  using runtime_error::runtime_error;
};

class LICE_IBitmap;
struct reaper_array;

#ifdef REAIMGUIAPI_IMPLEMENT
#  define REAIMGUIAPI_EXTERN
#else
#  define REAIMGUIAPI_EXTERN extern
#endif

namespace ImGui {
  constexpr const char *version = ")" << API_VERSION << R"(";
  void init(void *(*plugin_getapi)(const char *));

  namespace details {
    REAIMGUIAPI_EXTERN const char *(*last_error)() noexcept;

    inline void check_error()
    {
      if(const char *err {last_error()})
        throw ImGui_Error {err};
    }

    struct nullopt_t {
      constexpr explicit nullopt_t(int) {}
    };
    constexpr nullopt_t nullopt {0};

    template<typename T, typename E = void>
    class optional {
    public:
      using value_type = T*;
      optional(nullopt_t) : m_present {false} {}
      optional(const T v) : m_value {v}, m_present {true} {}
      optional(value_type) = delete;
      operator value_type() { return m_present ? &m_value : nullptr; }

    private:
      T m_value;
      bool m_present;
    };

    template<typename T>
    class optional<T, typename std::enable_if_t<std::is_pointer_v<T>>> {
    public:
      using value_type = T;
      optional(nullopt_t) : optional {nullptr} {}
      optional(T ptr) : m_value {ptr} {}
      operator value_type() { return m_value; }

    private:
      T m_value;
    };

    template<typename T> struct param { using value_type = T; };
    template<typename T> struct param<optional<T>> {
      using value_type = typename optional<T>::value_type;
    };

    template<typename T, bool nodiscard = false>
    class function;

    template<typename R, typename... Args, bool nodiscard>
    class function<R(Args...), nodiscard> {
    public:
      using Proc = R(*)(typename param<Args>::value_type...) noexcept;
      function() : m_proc {nullptr} {}
      function(Proc proc) : m_proc {proc} {}
      operator bool() const { return m_proc != nullptr; }

      template<typename... CallArgs, bool ND = nodiscard>
      std::enable_if_t<!ND, R> operator()(CallArgs&&... args) const
      {
        if constexpr(sizeof...(CallArgs) < sizeof...(Args))
          return (*this)(std::forward<CallArgs>(args)..., nullopt);
        else
          return invoke(std::forward<CallArgs>(args)...);
      }

      template<typename... CallArgs, bool ND = nodiscard> [[nodiscard]]
      std::enable_if_t<ND, R> operator()(CallArgs&&... args) const
      {
        if constexpr(sizeof...(CallArgs) < sizeof...(Args))
          return (*this)(std::forward<CallArgs>(args)..., nullopt);
        else
          return invoke(std::forward<CallArgs>(args)...);
      }

    protected:
      friend void ImGui::init(void *(*)(const char *));
      function(void *proc) : m_proc {reinterpret_cast<Proc>(proc)} {}

    private:
      R invoke(Args... args) const
      {
        if constexpr(std::is_void_v<R>) {
          m_proc(std::forward<Args>(args)...);
          check_error();
        }
        else {
          const R rv {m_proc(std::forward<Args>(args)...)};
          check_error();
          return rv;
        }
      }

      Proc m_proc;
    };

    inline int get_enum(const function<int()> f)
    {
      return f ? f() : 0;
    }
  }
)";

  for(const Function &func : g_funcs) {
    if(!(func.flags & API::Symbol::TargetNative))
      continue;

    stream << "\n  REAIMGUIAPI_EXTERN ";

    if(func.isEnum())
      stream << func.type << ' ';
    else {
      stream << "details::function<" << func.type << '(';
      CommaSep cs {stream};
      for(const Argument &arg : func.args) {
        if(arg.isOptional()) {
          cs << "details::optional<";
          if(!arg.isOutput() && arg.type.isScalarPtr())
            stream << arg.type.removePtr();
          else
            stream << arg.type;
          stream << '>';
        }
        else
          cs << arg.type;
        stream << ' ' << arg.name;
      }
      stream << ')';
      if(func.isNoDiscard())
        stream << ", true";
      stream << "> ";
    }

    stream << func.name << ';';
  }

  stream << R"(
}

#undef REAIMGUIAPI_EXTERN

#ifdef REAIMGUIAPI_IMPLEMENT
void ImGui::init(void *(*plugin_getapi)(const char *))
{
  void *(*get_func)(const char *v, const char *n) noexcept = reinterpret_cast<decltype(get_func)>(plugin_getapi("ImGui__getapi"));
  details::last_error = reinterpret_cast<decltype(details::last_error)>(plugin_getapi("ImGui__geterr"));
  if(!get_func || !details::last_error)
    throw ImGui_Error {"ReaImGui is not installed or too old"};
)";

  for(const Function &func : g_funcs) {
    if(!(func.flags & API::Symbol::TargetNative))
      continue;

    stream << "\n  " << func.name << " = ";
    if(func.isEnum())
      stream << "details::get_enum(";
    stream << "get_func(version, \"" << func.name << "\")";
    if(func.isEnum())
      stream << ')';
    stream << "; details::check_error();";
  }

  stream << R"(
}
#endif

#endif
)";
}

static std::string zigType(const Type type, bool isOptional = false)
{
  const auto noptr {type.removePtr()};

  std::string base;
  if(type.isString())
    base = type.isConst() ? "[*:0]const u8" : "[*]u8";
  else if(type.isPointer() && noptr.isVoid())
    base = "anyopaque";
  else if(noptr.isInt())
    base = "c_int";
  else if(noptr.isDouble())
    base = "f64";
  else if(noptr.isBool())
    base = "bool";
  else
    base = noptr;

  if(type.isPointer() && !type.isString()) {
    constexpr std::string_view prefix {"ImGui_"};
    if(base.substr(0, prefix.size()) == prefix) {
      base = base.substr(prefix.size()) + "Ptr";
      isOptional = false;
    }
    else
      base.insert(0, "*");
  }

  if(isOptional)
    base.insert(0, "?");

  return base;
}

static void zigBinding(std::ostream &stream)
{
  stream << "// " << GENERATED_FOR << R"(

const std = @import("std");

pub const api_version = ")" << API_VERSION << R"(";

pub const ContextPtr          = ?*opaque {};
pub const DrawListPtr         = ?*opaque {};
pub const DrawListSplitterPtr = ?*opaque {};
pub const FontPtr             = ?*opaque {};
pub const FunctionPtr         = ?*opaque {};
pub const ImagePtr            = ?*opaque {};
pub const ImageSetPtr         = ?*opaque {};
pub const ListClipperPtr      = ?*opaque {};
pub const ResourcePtr         = ?*opaque {};
pub const TextFilterPtr       = ?*opaque {};
pub const ViewportPtr         = ?*opaque {};
pub const LICE_IBitmap        =   anyopaque;
pub const reaper_array        =   anyopaque;

pub const Error = error { ImGui };
pub var last_error: ?[*:0]const u8 = null;

const API = struct {
)";

  for(const Function &func : g_funcs) {
    if(!(func.flags & API::Symbol::TargetNative) || func.isEnum())
      continue;

    stream << "  " << func.name << ": ";
    stream << "*fn(";
    CommaSep cs {stream};
    for(const Argument &arg : func.args)
      cs << zigType(arg.type, arg.isOptional());
    stream << ") callconv(.C) " << zigType(func.type) << ",\n";
  }

  stream << "\n";

  for(const Function &func : g_funcs) {
    if((func.flags & API::Symbol::TargetNative) && func.isEnum())
      stream << "  pub var " << func.name << ": c_int = undefined;\n";
  }

  stream << "};\n\nvar api: API = undefined;\npub usingnamespace API;\n\n";

  for(const Function &func : g_funcs) {
    if(!(func.flags & API::Symbol::TargetNative) || func.isEnum())
      continue;

    size_t minArgc {};
    for(const Argument &arg : func.args) {
      if(arg.isOptional())
        break;
      ++minArgc;
    }

    stream << "pub const " << func.name << " = function("
           << "&api." << func.name << ", " << minArgc << ", &.{";
    if(!func.args.empty())
      stream << ' ';
    CommaSep cs {stream};
    for(const Argument &arg : func.args) {
      if(arg.isOptional()) {
        if(!arg.isOutput() && arg.type.isScalarPtr())
          cs << zigType(arg.type.removePtr(), true);
        else
          cs << zigType(arg.type, true);
      }
      else
        cs << zigType(arg.type);
    }
    if(!func.args.empty())
      stream << ' ';
    stream << "});\n";
  }

  stream << R"(
var getError: ?*fn() callconv(.C) ?[*:0]const u8 = undefined;

inline fn checkError() Error!void {
  @setRuntimeSafety(false);
  last_error = getError.?();
  if(last_error != null)
    return error.ImGui;
}

inline fn getEnum(func: ?*fn() callconv(.C) c_int) c_int {
  return if(func) |f| f() else 0;
}

pub fn init(plugin_getapi: *fn(name: [*:0]const u8) callconv(.C) ?*anyopaque) !void {
  @setEvalBranchQuota(0x1000);
  @setRuntimeSafety(false);

  const getFunc: ?*fn(v: [*:0]const u8, n: [*:0]const u8) callconv(.C) *anyopaque =
    @ptrCast(plugin_getapi("ImGui__getapi"));
  getError = @ptrCast(plugin_getapi("ImGui__geterr"));

  if(getFunc == null or getError == null) {
    last_error = "ReaImGui is not installed or too old";
    return error.ImGui;
  }

  inline for(@typeInfo(API).@"struct".fields) |field| {
    @field(api, field.name) = @ptrCast(getFunc.?(api_version, field.name));
    try checkError();
  }

  inline for(@typeInfo(API).@"struct".decls) |decl| {
    @field(API, decl.name) = getEnum(@ptrCast(getFunc.?(api_version, decl.name)));
    try checkError();
  }
}

fn funcType(comptime func: anytype) type {
  return @typeInfo(@TypeOf(func.*)).pointer.child;
}

fn returnType(comptime func: anytype) type {
  return Error!@typeInfo(funcType(func)).@"fn".return_type.?;
}

fn function(comptime func: anytype, min_argc: comptime_int,
    comptime arg_types: []const type)
    fn(args: anytype) callconv(.Inline) returnType(func) {
  return struct {
    inline fn wrapper(args: anytype) returnType(func) {
      var cast_args: std.meta.Tuple(arg_types) = undefined;
      if(args.len < min_argc) {
        @compileError(std.fmt.comptimePrint("expected {}..{} arguments, got {}",
          .{min_argc, cast_args.len, args.len}));
      }
      inline for(0..cast_args.len) |i| {
        if(i >= args.len) {
          cast_args[i] = null;
          continue;
        }
        const arg_type = @typeInfo(@TypeOf(args[i]));
        comptime var cast_arg_type = @typeInfo(@TypeOf(cast_args[i]));
        if(cast_arg_type == .optional)
          cast_arg_type = @typeInfo(cast_arg_type.optional.child);
        cast_args[i] = if(cast_arg_type == .int and (
            (arg_type == .comptime_int and args[i] > std.math.maxInt(c_int)) or
            (arg_type == .int and arg_type.int.signedness == .unsigned)))
          @bitCast(@as(c_uint, args[i]))
        else
          args[i];
      }

      var call_args: std.meta.ArgsTuple(funcType(func)) = undefined;
      inline for(0..call_args.len) |i| {
        const cast_arg_type = @typeInfo(@TypeOf(cast_args[i]));
        call_args[i] =
          if(cast_arg_type == .optional)
            if(cast_args[i]) |*arg_val|
              if(@typeInfo(cast_arg_type.optional.child) == .pointer)
                arg_val.*
              else
                arg_val
            else
              null
          else
            cast_args[i];
      }

      const rv = @call(.auto, func.*, call_args);
      try checkError();
      return rv;
    }
  }.wrapper;
}
)";
}

std::string_view Argument::humanName() const
{
  size_t pos;
  if((pos = name.find("In")) && pos != std::string_view::npos)
    return name.substr(0, pos);
  else if((pos = name.find("Out")) && pos != std::string_view::npos)
    return name.substr(0, pos);
  return name;
}

enum class Highlight {
  End,
  Type,
  Reference,
  String,
  Constant,
};

static std::string hl(const Highlight type = Highlight::End)
{
  if(type == Highlight::End)
    return "</span>";

  std::string tag;
  tag += "<span class=\"";
  switch(type) {
  case Highlight::Type:
    tag += "st";
    break;
  case Highlight::Reference:
    tag += "sr";
    break;
  case Highlight::String:
    tag += "ss";
    break;
  case Highlight::Constant:
    tag += "sn";
    break;
  case Highlight::End:
    return "Invalid Highlight";
  }
  tag += "\">";
  return tag;
}

void Argument::defaultValue(std::ostream &stream, const char *null) const
{
  if(defv.empty())
    stream << hl(Highlight::Constant) << null;
  else if(defv[0] == '"')
    stream << hl(Highlight::String) << defv;
  else
    stream << hl(Highlight::Constant) << defv;
  stream << hl();
}

bool Function::hasOutputArgs() const
{
  for(const Argument &arg : args) {
    if(arg.isOutput())
      return true;
  }

  return false;
}

bool Function::hasOptionalArgs() const
{
  for(const Argument &arg : args) {
    if(arg.isOptional())
      return true;
  }

  return false;
}

void Function::cppSignature(std::ostream &stream) const
{
  stream << hl(Highlight::Type) << type << hl() << " ImGui::" << name;
  if(isEnum())
    return;
  stream << '(';
  CommaSep cs {stream};
  for(const Argument &arg : args) {
    cs << hl(Highlight::Type);
    if(arg.isOptional() && !arg.isOutput() && arg.type.isScalarPtr())
      stream << arg.type.removePtr();
    else
      stream << arg.type;
    stream << hl() << ' ' << arg.name;
    if(arg.isOptional()) {
      stream << " = ";
      arg.defaultValue(stream, "nullptr");
    }
  }
  stream << ')';
}

static std::string_view luaType(const Type type)
{
  if(type.isString())
    return "string";
  else if(type.removePtr().isInt())
    return "integer";
  else if(type.removePtr().isDouble())
    return "number";
  else if(type.removePtr().isBool())
    return "boolean";
  else if(type == "reaper_array*")
    return "reaper.array";
  else if(type.isPointer() && type.removePtr().isVoid())
    return "userdata"; // ValidatePtr
  else
    return type.removePtr();
}

void Function::luaSignature(std::ostream &stream) const
{
  bool hasReturns {false};
  {
    CommaSep cs {stream};
    if(!type.isVoid()) {
      cs << hl(Highlight::Type) << luaType(type) << hl() << ' ';
      if(!isEnum()) {
        stream << "retval";
        hasReturns = true;
      }
    }
    for(const Argument &arg : args) {
      if(arg.isOutput() && !arg.isBufSize()) {
        cs << hl(Highlight::Type) << luaType(arg.type) << hl();
        stream << ' ' << arg.humanName();
        hasReturns = true;
      }
    }
  }
  if(hasReturns)
    stream << " = ";
  stream << "ImGui." << name;
  if(isEnum())
    return;
  stream << '(';
  {
    const bool listOutputs {hasOptionalArgs()};
    CommaSep cs {stream};
    for(const Argument &arg : args) {
      if(arg.isBufSize())
        continue;
      else if(!arg.isInput()) {
        if(listOutputs)
          cs << hl(Highlight::Constant) << "nil" << hl();
        continue;
      }
      cs << hl(Highlight::Type) << luaType(arg.type)
         << hl() << ' ' << arg.humanName();
      if(arg.isOptional()) {
        stream << " = ";
        arg.defaultValue(stream, "nil");
      }
    }
  }
  stream << ')';
}

void Function::eelSignature(std::ostream &stream, const bool legacySyntax) const
{
  if(isVar()) {
    if(type.isString())
      stream << hl(Highlight::Reference) << '#' << name << hl();
    else if(type.isImVec2()) {
      stream << hl(Highlight::Type) << "double" << hl() << ' ';
      stream << name << ".x, " << name << ".y";
    }
    else
      stream << hl(Highlight::Type) << type << hl() << ' ' << name;

    return;
  }

  CommaSep cs {stream};
  if(!type.isVoid())
    stream << hl(Highlight::Type) << type << hl() << ' ';

  if(legacySyntax)
    cs << "extension_api(" << hl(Highlight::String) << "\"" API_PREFIX << name << '"' << hl();
  else {
    if(!(flags & API::Symbol::TargetEELFunc))
      stream << API_PREFIX;
    stream << name << '(';
  }
  for(const Argument &arg : args) {
    if(arg.isBufSize())
      continue;
    else if(arg.type.isString()) {
      if(arg.isOutput())
        cs << hl(Highlight::Reference) << '#' << arg.humanName() << hl();
      else
        cs << hl(Highlight::String) << '"' << arg.humanName() << '"' << hl();
    }
    else {
      cs << "";
      if(!arg.type.removePtr().isDouble())
        stream << hl(Highlight::Type) << arg.type.removePtr() << hl() << ' ';
      if(arg.isOutput())
        stream << hl(Highlight::Reference) << "&amp;" << hl();
      stream << arg.humanName();
    }
    if(arg.isOptional()) {
      stream << " = ";
      arg.defaultValue(stream, "0");
    }
  }
  stream << ')';
}

static std::string_view pythonType(const Type type)
{
  if(type.isString())
    return "str";
  else if(type.removePtr().isInt())
    return "int";
  else if(type.removePtr().isDouble())
    return "float";
  else
    return type.removePtr();
}

void Function::pythonSignature(std::ostream &stream) const
{
  if(hasOutputArgs()) {
    CommaSep cs {stream};
    stream << '(';
    if(!type.isVoid())
      cs << hl(Highlight::Type) << pythonType(type) << hl() << " retval";
    for(const Argument &arg : args) {
      if(!arg.isOutput() || arg.isBufSize())
        continue;
      cs << hl(Highlight::Type) << pythonType(arg.type)
         << hl() << ' ' << arg.humanName();
    }
    stream << ") = ";
  }
  else if(!type.isVoid())
    stream << hl(Highlight::Type) << pythonType(type) << hl() << " retval = ";

  stream << "ImGui." << name << '(';
  {
    CommaSep cs {stream};
    for(const Argument &arg : args) {
      if(arg.isBufSize() || !arg.isInput())
        continue;
      cs << hl(Highlight::Type) << pythonType(arg.type)
         << hl() << ' ' << arg.humanName();
      if(arg.isOptional()) {
        stream << " = ";
        arg.defaultValue(stream, "None");
      }
    }
  }
  stream << ')';
}

static auto findNewSection
  (const Function &func, std::vector<const API::Section *> &oldSections)
{
  auto oldSection {oldSections.begin()};
  auto newSection {func.sections.begin()};
  while(oldSection != oldSections.end() && newSection != func.sections.end() &&
        *oldSection == *newSection)
    ++oldSection, ++newSection;
  oldSections.erase(oldSection, oldSections.end());
  std::copy(newSection, func.sections.end(), std::back_inserter(oldSections));
  return newSection;
}

static void outputHtmlText(std::ostream &stream, std::string_view text)
{
  // outputting char by char is slower than as many as possible at once
  while(!text.empty()) {
    const size_t nextEntity {text.find_first_of("<>&")};
    stream << text.substr(0, nextEntity);
    if(nextEntity == std::string_view::npos)
      return;

    switch(text[nextEntity]) {
    case '<':
      stream << "&lt;";
      break;
    case '>':
      stream << "&gt;";
      break;
    case '&':
      stream << "&amp;";
      break;
    }
    text.remove_prefix(nextEntity + 1);
  }
}

struct Reference {
  const Function *func;
  std::string_view range;
};

static std::vector<Reference> parseReferences(const std::string_view &input)
{
  // build a map of known references for fast lookup
  enum CharInfo { InitialChar = 1<<0, ValidChar = 1<<1 };
  static std::map<std::string_view, const Function *> funcs;
  static char charmap[0x100];
  if(funcs.empty()) {
    charmap[static_cast<size_t>('*')] |= ValidChar;
    for(const Function &func : g_funcs)
      funcs.emplace(func.name, &func);
    // build a maps of which characters may be present in a reference
    for(const auto &pair : funcs) {
      charmap[static_cast<unsigned char>(pair.first[0])] |= InitialChar;
      for(const unsigned char c : pair.first)
        charmap[c] |= ValidChar;
    }
  }

  std::vector<Reference> links;

  auto start {input.begin()};
  while(start != input.end()) {
    start = std::find_if(start, input.end(),
      [&](const unsigned char c) { return charmap[c] & InitialChar; });
    if(start == input.end())
      break;
    const auto end {std::find_if_not(start, input.end(),
      [&](const unsigned char c) { return charmap[c] & ValidChar; })};
    // constructor taking (first, last) iterators is C++20
    const std::string_view word {&*start, static_cast<size_t>(end - start)};
    decltype(funcs)::const_iterator it;
    if(*word.rbegin() == '*') {
      const std::string_view prefix {word.substr(0, word.size() - 1)};
      it = funcs.lower_bound(prefix);
      // starts_with is C++20
      if(it->first.substr(0, prefix.size()) != prefix)
        it = funcs.end();
    }
    else
      it = funcs.find(word);
    if(it != funcs.end())
      links.push_back({it->second, word});
    start += word.size();
  }

  return links;
}

static void outputHtmlBlock(std::ostream &stream, std::string_view html,
  const bool escape = true)
{
  const auto &links {parseReferences(html)};
  for(auto link {links.begin()}; link != links.end(); ++link) {
    const auto prefixSize {link->range.data() - html.data()};
    const std::string_view prefix {html.substr(0, prefixSize)};
    if(escape)
      outputHtmlText(stream, prefix);
    else
      stream << prefix;
    stream << "<a href=\"#" << link->func->name << "\">"
           << link->range << "</a>";
    html.remove_prefix(prefixSize + link->range.size());
  }
  if(escape)
    outputHtmlText(stream, html);
  else
    stream << html;
}

static void outputMarkdown(const char *data, MD_SIZE size, void *userData)
{
  std::ostream &stream {*static_cast<std::ostream *>(userData)};
  outputHtmlBlock(stream, {data, size}, false);
}

static void outputMarkdown(std::ostream &stream, const std::string_view &text)
{
  constexpr auto parserFlags
    {MD_FLAG_NOHTML | MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_TABLES};
  if(md_html(text.data(), text.size(), &outputMarkdown, &stream, parserFlags, 0)) {
    stream << "<pre>";
    outputHtmlBlock(stream, text);
    stream << "</pre>";
  }
}

static void outputHtmlSlug(std::ostream &stream, const std::string_view &text)
{
  bool prevWasPrintable {false};
  for(const char c : text) {
    if(isalnum(c)) {
      stream << static_cast<char>(tolower(c));
      prevWasPrintable = true;
    }
    else if(prevWasPrintable) {
      stream << '-';
      prevWasPrintable = false;
    }
  }
}

static void outputSectionSlug(std::ostream &stream,
  const Function &func, std::deque<const API::Section *>::const_iterator section)
{
  for(auto it {func.sections.begin()}; it <= section; ++it) {
    if(it != func.sections.begin())
      stream << '-';
    outputHtmlSlug(stream, (*it)->title);
  }
}

static void humanBinding(std::ostream &stream)
{
  stream << R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <title>ReaImGui Documentation</title>
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/hack-font@3.3.0/build/web/hack-subset.css"/>
  <style>
  body {
    background-color: #0D0D0D;
    color: #d9d3d3;
    font-size: 15px;
    line-height: 1.33;
    margin: 0;
    overflow-anchor: none;
  }
  body, pre, code {
    font-family: Menlo, "DejaVu Sans Mono", Hack, Consolas, monospace;
  }
  aside {
    background-color: #262626; /* Grey15 */
    border-right: 1px solid #6f6f6f;
    bottom: 0;
    overflow-y: auto;
    position: fixed;
    top: 0;
    width: 200px;
  }
  aside ol li { display: block; }
  aside ol li::before { display: none; }
  aside li a {
    display: block;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  main {
    margin-left: 200px;
    margin-right: 1em;
    box-shadow: 0 0 10px #080808;
  }
  aside p, aside li a, main { padding-left: 1em; }
  h1, h2, h3, h4, h5, h6, hr, pre, table { margin: 1rem 0 1rem 0; }
  h1 { font-size: 2.3em;  }
  h2 { font-size: 1.8em;  }
  h3 { font-size: 1.4em;  }
  h4 { font-size: 1.15em; }
  h5 { font-size: 1.0em;  }
  h6 { font-size: 0.9em;  }
  h2::before { content: '〉'; color: #6f6f6f; }
  hr { border: none; border-top: 1px solid #6f6f6f; }
  ol {
    padding: 0;
    list-style-type: none;
    counter-reset: item;
  }
  ol > li { display: table; }
  ol > li::before {
    content: counter(item) ".";
    counter-increment: item;
    display: table-cell;
    padding-right: 0.5em;
  }
  ol ol > li::before { content: counters(item, "."); }
  ul { padding-left: 1em; }
  li ul { list-style-type: square; }
  a { text-decoration: none; }
  a, summary { color: #00ff87; /* SpringGreen1 */ }
  .toc { columns: 24em 6; }
  .toc ol { margin:  0; }
  .toc > ol > li > a { font-weight: bold; }
  a:hover, summary:hover { text-decoration: underline; }
  details { margin-left: 20px; }
  summary {
    cursor: pointer;
    display: inline-block;
    list-style-type: none;
  }
  summary::before { content: '+ '; margin-left: -20px; }
  summary + table, summary + p { margin-top: 0; }
  details[open] summary::before { content: '- '; }
  summary::-webkit-details-marker { display: none; }
  pre { white-space: pre-wrap; }
  pre code {
    background-color: black;
    border-radius: 5px;
    border: 1px solid #6f6f6f;
    display: inline-block;
    padding: 0.5em;
  }
  code, code a { color: white; }
  table { border-collapse: collapse; }
  th {
    padding-left: 0;
    padding-right: 0.5em;
    text-align: left;
    vertical-align: top;
    white-space: nowrap;
  }
  table code:hover { text-decoration: underline; cursor: copy; }
  table code:active, aside a:hover { background-color: #3a3a3a; }
  tr + tr td { border-top: 1px solid #555; }
  .st { color: #87afff; /* SkyBlue2 */ }
  .ss, .string   { color: #5faf5f; /* DarkSeaGreen4 */ }
  .sn, .number   { color: #5f87d7; /* SteelBlue3    */ }
  .sr, .keyword  { color: #d7875f; /* LightSalmon3  */ }
       .comment  { color: #b2b2b2; /* Grey70        */ }
       .built_in { color: #87d75f; /* DarkOliveGreen3 */ }
  .meta, .meta a { color: gray; }
  </style>
</head>
<body>
  <aside>
    <p><strong>Quick Jump</strong></p>
    <ol><li><a href="#toc">Table of Contents</a></li></ol>
    <ol>)";

  const API::Section *section {};
  for(const Function &func : g_funcs) {
    if(func.sections[0] == section)
      continue;

    section = func.sections[0];

    stream << "<li><a href=\"#";
    outputHtmlSlug(stream, section->title); stream << "\">";
    outputHtmlText(stream, section->title);
    stream << "</a></li>";
  }

  stream << R"(
    </ol>
  </aside>
  <main>
    <h1>ReaImGui Documentation</h1>
    <p>Generated for version )" << REAIMGUI_VERSION;
  stream << " &middot; API version " << API_VERSION << R"(</p>

    <p>
      <a href="https://forum.cockos.com/showthread.php?t=250419">Forum thread</a>
      &middot; <a href="https://github.com/cfillion/reaimgui">Source repository</a>
      &middot; <a href="https://github.com/cfillion/reaimgui/issues">Issue tracker</a>
    </p><hr/>)";

  std::vector<const API::Section *> sections;
  stream << "<h2 id=\"toc\">Table of Contents</h2>";
  stream << "<div class=\"toc\">";
  long long level {-1};
  for(const Function &func : g_funcs) {
    for(auto it {findNewSection(func, sections)};
        it != func.sections.end(); ++it) {
      const API::Section *section {*it};
      const auto thisLevel {std::distance(func.sections.begin(), it) + 1};

      if(thisLevel == level)
        stream << "</li>";
      if(thisLevel > level) {
        stream << "<ol>";
        level = thisLevel;
      }
      while(thisLevel < level) {
        stream << "</li></ol>";
        --level;
      }

      stream << "<li><a href=\"#";
      outputSectionSlug(stream, func, it);
      stream << "\">";
      outputHtmlText(stream, section->title);
      stream << "</a>";
    }
  }
  while(level-- > 0)
    stream << "</li></ol>";
  stream << "</div>\n\n";

  for(const Function &func : g_funcs) {
    for(auto it {findNewSection(func, sections)};
        it != func.sections.end(); ++it) {
      const API::Section *section {*it};
      const auto level {std::distance(func.sections.begin(), it) + 2};

      if(level == 2)
        stream << "<hr/>";
      stream << "<h" << level << " id=\"";
      outputSectionSlug(stream, func, it);
      stream << "\">";
      outputHtmlText(stream, section->title);
      stream << "</h" << level << '>';

      if(strlen(section->help))
        outputMarkdown(stream, section->help);
    }

    stream << "<details id=\"" << func.name << "\"><summary>";
    if(func.isVar())
      stream << "Variable: ";
    else if(func.isEnum())
      stream << "Constant: ";
    else
      stream << "Function: ";
    stream << func.name << "</summary>";

    struct Target {
      const char *name;
      int flag;
      std::function<void(const Function *, std::ostream &)> formatter;
    };
    using namespace std::placeholders;
    static const Target targets[] {
      {"C++",        API::Symbol::TargetNative, std::mem_fn(&Function::cppSignature)            },
      {"EEL",        API::Symbol::TargetScript | API::Symbol::TargetEELFunc,
       std::bind(&Function::eelSignature, _1, _2, false)},
      {"Legacy EEL", API::Symbol::TargetScript, std::bind(&Function::eelSignature, _1, _2, true)},
      {"Lua",        API::Symbol::TargetScript, std::mem_fn(&Function::luaSignature)            },
      {"Python",     API::Symbol::TargetScript, std::mem_fn(&Function::pythonSignature)         },
    };
    stream << "<table>";
    for(const Target &target : targets) {
      if(!(func.flags & target.flag))
        continue;
      stream << "<tr><th>" << target.name << "</th><td><code>";
      target.formatter(&func, stream);
      stream << "</code></td></tr>";
    }
    stream << "</table>";

    if(!func.doc.empty())
      outputMarkdown(stream, func.doc);

    stream << "<p class=\"meta\">"
              "<a href=\"https://github.com/cfillion/reaimgui/blob/v"
              REAIMGUI_VERSION "/api/" << func.section->file << ".cpp#L"
           << func.line << "\">View source</a>"
           << " &middot; v" << func.version.toString() << "+</p>";

    stream << "</details>\n";
  }

  stream << R"(<p>EOF</p>
  </main>

  <script>
  function openTarget() {
    var hash = location.hash.substring(1);
    var target = hash && document.getElementById(hash);
    if(target && target.tagName.toLowerCase() === 'details') {
      target.open = true;
      target.scrollIntoView();
    }
  }

  window.addEventListener('hashchange', openTarget);
  openTarget();

  document.body.addEventListener('click', function(e) {
    if(e.target.tagName.toLowerCase() === 'summary' && !e.target.parentNode.open)
      history.replaceState(null, '', location.pathname + '#' + e.target.parentNode.id);
    else if(e.target.tagName.toLowerCase() === 'code')
      navigator.clipboard.writeText(e.target.textContent);
  });
  </script>
  <!-- highlight.js v11 removed auto-merging of html (removes links) -->
  <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/10.7.3/highlight.min.js" integrity="sha512-tL84mD+FR70jI7X8vYj5AfRqe0EifOaFUapjt1KvDaPLHgTlUZ2gQL/Tzvvn8HXuQm9oHYShJpNFdyJmH2yHrw==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
  <script>
  hljs.configure({
    classPrefix: '',
    languages: ['lua', 'cpp'],
  });
  hljs.highlightAll();
  </script>
</body>
</html>
)";
}

static void outputEscapedMarkdown(std::ostream &stream, const std::string_view &text)
{
  for(const char c : text) {
    if(ispunct(c))
      stream << '\\';
    stream << c;
  }
}

static void luaLSBlock(std::ostream &stream, std::string_view markdown)
{
  while(!markdown.empty()) {
    size_t endl {markdown.find('\n')};
    const std::string_view line {markdown.substr(0, endl)};
    if(line.empty())
      stream << "---";
    else
      stream << "--- " << line;
    if(endl == std::string_view::npos || endl == markdown.size() - 1)
      break;
    stream << '\n';
    markdown.remove_prefix(endl + 1);
  }
}

static void luaLSAnnotate(std::ostream &stream, const Function &func)
{
  constexpr const char *separator {"\n---\n--- ---\n---\n"};

  stream << "--- **";
  CommaSep heading {stream, " > "};
  for(const API::Section *section : func.sections) {
    heading << "";
    outputEscapedMarkdown(stream, section->title);
  }
  heading << "";
  outputEscapedMarkdown(stream, func.name);
  stream << "**";

  if(!func.doc.empty()) {
    stream << "\n---\n";
    luaLSBlock(stream, func.doc);
  }
  stream << separator;

  for(auto it = func.sections.rbegin(); it < func.sections.rend(); ++it) {
    const API::Section *section {*it};
    if(!strlen(section->help))
      continue;
    stream << "--- **";
    CommaSep heading {stream, " > "};
    for(const API::Section *parent : func.sections) {
      heading << "";
      outputEscapedMarkdown(stream, parent->title);
      if(parent == section)
        break;
    }
    stream << "**\n---\n";
    luaLSBlock(stream, section->help);
    stream << separator;
  }

  // @since is not a standard annotation
  stream << "--- @since " << func.version.toString() << '\n';

  if(func.isEnum())
    return;

  const bool listOutputs {func.hasOptionalArgs()};
  size_t skipCount {};
  for(const Argument &arg : func.args) {
    if(arg.isBufSize())
      continue;
    else if(!arg.isInput()) {
      // '?' required to enable type validation (accepts anything otherwise)
      // zero-width space trick required to not display 'any' as the param type
      if(listOutputs)
        stream << "--- @param _" << ++skipCount << "? nil​\n";
      continue;
    }

    stream << "--- @param " << arg.humanName();
    if(arg.isOptional())
      stream << '?';
    stream << ' ' << luaType(arg.type);
    if(arg.isOptional()) {
      const std::string_view defv {arg.defv.empty() ? "nil" : arg.defv};
      stream << " default value = `" << defv << '`';
    }
    else if(arg.type == "ImGui_Font*")
      stream << "|nil"; // default font special case
    stream << '\n';
  }

  if(!func.type.isVoid()) {
    if(func.isNoDiscard())
      stream << "--- @nodiscard\n";
    stream << "--- @return " << luaType(func.type) << " retval\n";
  }
  for(const Argument &arg : func.args) {
    if(!arg.isOutput() || arg.isBufSize())
      continue;
    stream << "--- @return " << luaType(arg.type) << ' ' << arg.humanName() << '\n';
  }
}

static void luaLSBinding(std::ostream &stream)
{
  stream << "--- ReaImGui LuaCATS definitions\n---\n"
         << "--- Generated for version " REAIMGUI_VERSION
         << " - API version " << API_VERSION << "\n---\n"
         << "--- @meta  imgui\n--- @class ImGui\n";

  for(const Function &func : g_funcs) {
    if(!(func.flags & API::Symbol::TargetScript) | !func.isEnum())
      continue;

    stream << "---\n";
    luaLSAnnotate(stream, func);
    stream << "--- @field " << func.name << ' ' << luaType(func.type) << '\n';
  }

  // Disabling keyword diagnostics because some parameter names are
  // reserved Lua keywords (eg. 'repeat')
  stream << R"(local ImGui = {}

--- @alias nil​ nil
--- @class (exact) ImGui_Resource         : userdata
--- @class (exact) ImGui_DrawList         : userdata
--- @class (exact) ImGui_Viewport         : userdata
--- @class (exact) ImGui_Context          : ImGui_Resource
--- @class (exact) ImGui_DrawListSplitter : ImGui_Resource
--- @class (exact) ImGui_Font             : ImGui_Resource
--- @class (exact) ImGui_Function         : ImGui_Resource
--- @class (exact) ImGui_Image            : ImGui_Resource
--- @class (exact) ImGui_ImageSet         : ImGui_Image
--- @class (exact) ImGui_ListClipper      : ImGui_Resource
--- @class (exact) ImGui_TextFilter       : ImGui_Resource
--- @class (exact) LICE_IBitmap           : userdata
--- @diagnostic disable: keyword
)";

  for(const Function &func : g_funcs) {
    if(!(func.flags & API::Symbol::TargetScript) || func.isEnum())
      continue;

    stream << '\n';
    luaLSAnnotate(stream, func);
    stream << "function ImGui." << func.name << '(';
    const bool listOutputs {func.hasOptionalArgs()};
    size_t skipCount {};
    CommaSep cs {stream};
    for(const Argument &arg : func.args) {
      if(arg.isBufSize())
        continue;
      else if(!arg.isInput()) {
        if(listOutputs)
          cs << '_' << ++skipCount;
        continue;
      }
      cs << arg.humanName();
    }
    stream << ") end\n";
  }

  stream << R"(
--- @param api_version string
--- @return ImGui
return function(api_version) end
)";
}

static const char *pythonCType(const Type &type)
{
  static const std::unordered_map<std::string_view, const char *> ctypes {
    {"void",   "None"    },
    {"bool",   "c_bool"  },
    {"int",    "c_int"   },
    {"double", "c_double"},
  };

  if(type.isString())
    return "c_char_p";
  else if(type.isPointer())
    return "c_void_p";

  return ctypes.at(type);
}

static const char *pythonScalarType(const Type &type) // non-pointers scalars only
{
  static const std::unordered_map<std::string_view, const char *> pytypes {
    {"bool",   "int"  },
    {"int",    "int"  },
    {"double", "float"},
  };

  return pytypes.at(type);
}

static void pythonBinding(std::ostream &stream)
{
  stream << "# " << GENERATED_FOR << "\n\n"
            "from reaper_python import *\n";

  for(const Function &func : g_funcs) {
    if(!(func.flags & API::Symbol::TargetScript))
      continue;

    stream << "\ndef " << func.name << '(';
    {
      CommaSep cs {stream};
      for(const Argument &arg : func.args) {
        if(arg.isBufSize() || !arg.isInput())
          continue;
        cs << arg.name;
        if(arg.isOptional())
          stream << " = None";
      }
    }
    stream << "):\n"
              "  if not hasattr(" << func.name << ", 'func'):\n"
              "    proc = rpr_getfp('" API_PREFIX << func.name << "')\n"
              "    " << func.name << ".func = CFUNCTYPE(";
    {
      CommaSep cs {stream};
      cs << pythonCType(func.type);
      for(const Argument &arg : func.args)
        cs << pythonCType(arg.type);
    }
    stream << ")(proc)\n";

    if(!func.args.empty()) {
      stream << "  args = (";
      CommaSep cs {stream};
      for(const Argument &arg : func.args) {
        if(arg.type.isScalarPtr())
          cs << pythonCType(arg.type.removePtr());
        else if(arg.type.isString())
          cs << (arg.type.isConst() ? "rpr_packsc" : "rpr_packs");
        else
          cs << pythonCType(arg.type);

        stream << '(';
        if(arg.isBufSize()) {
          if(arg.isOutput())
            stream << (arg.isBigBuf() ? "4096" : "1024");
          else
            stream << "len(" << arg.bufName() << ")+1";
        }
        else if(arg.isInput())
          stream << arg.name;
        else
          stream << '0';
        stream << ')';
        if(arg.isOptional())
          stream << " if " << arg.name << " != None else None";
      }
      if(func.args.size() == 1)
        stream << ',';
      stream << ")\n";
    }

    if(func.isEnum()) {
      stream << "  if not hasattr(" << func.name << ", 'cache'):\n"
                "    " << func.name << ".cache = " << func.name << ".func()\n"
                "  return " << func.name << ".cache\n";
      continue;
    }

    {
      stream << "  ";
      if(!func.type.isVoid())
        stream << "rval = ";
      stream << func.name << ".func(";
      CommaSep cs {stream};
      for(size_t i {0}; i < func.args.size(); ++i) {
        const Argument &arg {func.args[i]};
        if(arg.type.isScalarPtr()) {
          cs << "byref(args[" << i << "])";
          if(arg.isOptional())
            stream << " if args[" << i << "] != None else None";
        }
        else
          cs << "args[" << i << ']';
      }
      stream << ")\n";
    }

    if(!func.type.isVoid() || func.hasOutputArgs()) {
      stream << "  return ";
      CommaSep cs {stream};
      if(!func.type.isVoid()) {
        if(func.type.isString())
          cs << "str(rval.decode())";
        else
          cs << "rval";
      }
      for(size_t i {0}; i < func.args.size(); ++i) {
        const Argument &arg {func.args[i]};
        if(!arg.isOutput() || arg.isBufSize())
          continue;
        else if(arg.type.isScalarPtr())
          cs << pythonScalarType(arg.type.removePtr())
            << "(args[" << i << "" << "].value)";
        else if(arg.type.isString() && !arg.type.isConst())
          cs << "rpr_unpacks(" << "args[" << i << "])";
        else
          cs << arg.name;
        if(arg.isOptional())
          stream << " if " << arg.name << " != None else None";
      }
      stream << '\n';
    }
  }
}

int main(int argc, const char *argv[])
{
  if(argc < 2) {
    std::cerr << "usage: reaper_extension.so [language]\n";
    return 1;
  }

  FuncImport<decltype(API_head)> _API_head {WIDEN(argv[1]), "API_head"};
  FuncImport<decltype(API_version)> _API_version {WIDEN(argv[1]), "API_version"};
  if(!_API_head || !_API_version) {
    std::cerr << "failed to find API head of '" << argv[1] << "'\n";
    return 2;
  }

  _API_version(API_VERSION, sizeof(API_VERSION));

  for(const API::Symbol *func {_API_head()}; func; func = func->m_next) {
    if(func->name()[0] != '_')
      g_funcs.push_back(func);
  }
  std::sort(g_funcs.begin(), g_funcs.end());

  const std::string_view lang {argc >= 3 ? argv[2] : "cpp"};

  if(lang == "cpp")
    cppBinding(std::cout);
  else if(lang == "zig")
    zigBinding(std::cout);
  else if(lang == "human")
    humanBinding(std::cout);
  else if(lang == "luals")
    luaLSBinding(std::cout);
  else if(lang == "python")
    pythonBinding(std::cout);
  else {
    std::cerr << "don't know how to generate a binding for '" << lang << "'\n";
    return 1;
  }

  return 0;
}
