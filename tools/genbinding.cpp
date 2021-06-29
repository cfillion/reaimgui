#include "version.hpp"

#include <algorithm>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <reaper_plugin.h>

constexpr const char *GENERATED_FOR { "Generated for ReaImGui v" REAIMGUI_VERSION };

extern "C" int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE, reaper_plugin_info_t *);

struct Type {
  Type(const char *val)             : m_value { val } {}
  Type(const std::string_view &val) : m_value { val } {}

  bool isVoid() const { return m_value == "void"; }
  bool isInt() const { return m_value == "int"; }
  bool isBool() const { return m_value == "bool"; }
  bool isDouble() const { return m_value == "double"; }
  bool isString() const { return m_value == "const char*" || m_value == "char*"; }
  bool isPointer() const { return m_value.size() >= 1 && m_value.back() == '*'; }
  bool isConst() const { return m_value.find("const ") == 0; }
  bool isNumber() const { return isInt() || isDouble(); };
  bool isScalar() const { return isBool() || isNumber(); }
  bool isScalarPtr() const { return isPointer() && removePtr().isScalar(); }

  Type removePtr() const
  {
    if(isPointer())
      return { std::string_view { m_value.data(), m_value.size() - 1 } };
    else
      return *this;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Type &t)
  {
    stream << t.m_value;
    return stream;
  }

  operator const std::string_view &() const { return m_value; }

private:
  std::string_view m_value;
};

struct Argument {
  Type type;
  std::string_view name;

  bool isOutBuffer(bool exclusive = false) const {
    return (!exclusive || name.find("In")  == std::string_view::npos) &&
           name.find("Out") != std::string_view::npos &&
           (exclusive || name.find("_sz") != name.size() - 3);
  }

  bool isOptional() const {
    return name.find("Optional") != std::string_view::npos;
  }

  bool isBufSize() const {
    return name.find("Out") != std::string_view::npos &&
           name.find("_sz") == name.size() - 3;
  }

  std::string_view humanName() const;
};

struct Function {
  std::string_view name;
  Type type;
  std::vector<Argument> args;
  std::string_view doc;

  bool isEnum() const { return type.isInt() && args.empty(); }
  bool hasOutBuffers() const;
  bool operator<(const Function &o) const { return name < o.name; }

  void cppSignature(std::ostream &) const;
  void luaSignature(std::ostream &) const;
  void eelSignature(std::ostream &, bool legacySyntax = false) const;
  void pythonSignature(std::ostream &) const;
};

static std::vector<Function> g_funcs;

static void addFunc(const char *name, const char *def)
{
  Function func { name, def };

  std::string_view argTypes { def += strlen(def) + 1 };
  std::string_view argNames { def += strlen(def) + 1 };
  func.doc = { def += strlen(def) + 1 };

  while(argTypes.size() > 0 && argNames.size() > 0) {
    size_t typeLen { argTypes.find(',') },
           nameLen { argNames.find(',') };

    func.args.emplace_back(Argument {
      argTypes.substr(0, typeLen),
      argNames.substr(0, nameLen)
    });

    if(typeLen == std::string_view::npos || nameLen == std::string_view::npos)
      break;

    argTypes.remove_prefix(typeLen + 1);
    argNames.remove_prefix(nameLen + 1);
  }

  g_funcs.push_back(func);
}

struct CommaSep {
  CommaSep(std::ostream &stream) : m_stream { stream }, m_sep { false } {}

  template<typename T>
  std::ostream &operator<<(const T rhs)
  {
    if(m_sep)
      m_stream << ", ";
    else
      m_sep = true;

    m_stream << rhs;
    return m_stream;
  }

private:
  std::ostream &m_stream;
  bool m_sep;
};

static void cppBinding(std::ostream &stream)
{
  stream << "// " << GENERATED_FOR << R"(

#ifndef REAPER_IMGUI_FUNCTIONS_H
#define REAPER_IMGUI_FUNCTIONS_H

#include <reaper_plugin_functions.h>
#include <utility>

class ImGui_Context;
class ImGui_DrawList;
class ImGui_Font;
class ImGui_ListClipper;
class ImGui_Viewport;

struct reaper_array;

template<typename T>
class ReaImGuiFunc;

template<typename R, typename... Args>
class ReaImGuiFunc<R(Args...)>
{
public:
  ReaImGuiFunc(const char *name) : m_name { name }, m_proc { nullptr } {}
  operator bool() const { return proc() != nullptr; }
  template<typename... CallArgs>
  auto operator()(CallArgs... args)
  {
    if constexpr(sizeof...(CallArgs) < sizeof...(Args))
      return (*this)(std::forward<CallArgs>(args)..., nullptr);
    else
      return proc()(std::forward<CallArgs>(args)...);
  }

private:
  R(*proc())(Args...)
  {
    if(!m_proc)
      m_proc = reinterpret_cast<decltype(m_proc)>(plugin_getapi(m_name));
    return m_proc;
  }

  const char *m_name;
  R(*m_proc)(Args...);
};

class ReaImGuiEnum
{
public:
  ReaImGuiEnum(const char *name) : m_name { name }, m_init { false } {}
  operator int()
  {
    if(!m_init) {
      ReaImGuiFunc<int()> func { m_name };
      m_value = func();
      m_init  = true;
    }
    return m_value;
  }

private:
  const char *m_name;
  bool m_init;
  int m_value;
};

#ifdef REAIMGUIAPI_IMPLEMENT
#  define REAIMGUIAPI_EXTERN
#  define REAIMGUIAPI_INIT(n) { n }
#else
#  define REAIMGUIAPI_EXTERN extern
#  define REAIMGUIAPI_INIT(n)
#endif

)";

  for(const Function &func : g_funcs) {
    stream << "REAIMGUIAPI_EXTERN ";

    if(func.isEnum())
      stream << "ReaImGuiEnum ";
    else {
      stream << "ReaImGuiFunc<" << func.type << '(';
      CommaSep cs { stream };
      for(const Argument &arg : func.args) {
        cs << arg.type << ' ' << arg.name;
      }
      stream << ")> ";
    }

    stream << func.name << " REAIMGUIAPI_INIT(\"" << func.name << "\");\n";
  }

  stream << R"(
#undef REAIMGUIAPI_EXTERN
#undef REAIMGUIAPI_INIT

#endif)" << std::endl;
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
    tag += "sc";
    break;
  case Highlight::End:
    return "Invalid Highlight";
  }
  tag += "\">";
  return tag;
}

bool Function::hasOutBuffers() const
{
  for(const Argument &arg : args) {
    if(arg.isOutBuffer())
      return true;
  }

  return false;
}

void Function::cppSignature(std::ostream &stream) const
{
  stream << hl(Highlight::Type) << type << hl() << ' ' << name << '(';
  CommaSep cs { stream };
  for(const Argument &arg : args) {
    cs << hl(Highlight::Type) << arg.type << hl() << ' ' << arg.name;
    if(arg.isOptional())
      stream << " = " << hl(Highlight::Constant) << "nullptr" << hl();
  }
  stream << ')';
}

static std::string_view luaType(const Type type)
{
  if(type.isString())
    return "string";
  else if(type.removePtr().isNumber())
    return "number";
  else if(type.removePtr().isBool())
    return "boolean";
  else
    return type.removePtr();
}

void Function::luaSignature(std::ostream &stream) const
{
  bool hasReturns { false };
  {
    CommaSep cs { stream };
    if(!type.isVoid()) {
      cs << hl(Highlight::Type) << luaType(type) << hl() << " retval";
      hasReturns = true;
    }
    for(const Argument &arg : args) {
      if(arg.isOutBuffer()) {
        cs << hl(Highlight::Type) << luaType(arg.type) << hl();
        stream << ' ' << arg.humanName();
        hasReturns = true;
      }
    }
  }
  if(hasReturns)
    stream << " = ";
  stream << "reaper." << name << '(';
  {
    CommaSep cs { stream };
    for(const Argument &arg : args) {
      if(arg.isBufSize())
        continue;
      cs << hl(Highlight::Type) << luaType(arg.type)
         << hl() << ' ' << arg.humanName();
      if(arg.isOutBuffer() || arg.isOptional())
        stream << " = " << hl(Highlight::Constant) << "nil" << hl();
    }
  }
  stream << ')';
}

void Function::eelSignature(std::ostream &stream, const bool legacySyntax) const
{
  CommaSep cs { stream };
  if(!type.isVoid())
    stream << hl(Highlight::Type) << type << hl() << ' ';

  if(legacySyntax)
    cs << "extension_api(" << hl(Highlight::String) << '"' << name << '"' << hl();
  else
    stream << name << '(';
  for(const Argument &arg : args) {
    if(arg.isBufSize())
      continue;
    if(arg.type.isString()) {
      if(arg.isOutBuffer())
        cs << hl(Highlight::Reference) << '#' << arg.humanName() << hl();
      else
        cs << hl(Highlight::String) << '"' << arg.humanName() << '"' << hl();
    }
    else {
      cs << hl(Highlight::Type) << arg.type.removePtr() << hl() << ' ';
      if(arg.isOutBuffer())
        stream << hl(Highlight::Reference) << "&amp;" << hl();
      stream << arg.humanName();
    }
    if(arg.isOptional())
      stream << " = " << hl(Highlight::Constant) << '0' << hl();
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
  if(hasOutBuffers()) {
    CommaSep cs { stream };
    stream << '(';
    if(!type.isVoid())
      cs << hl(Highlight::Type) << pythonType(type) << hl() << " retval";
    for(const Argument &arg : args) {
      if(!arg.isOutBuffer())
        continue;

      cs << hl(Highlight::Type) << pythonType(arg.type)
         << hl() << ' ' << arg.humanName();
    }
    stream << ") = ";
  }
  else if(!type.isVoid())
    stream << hl(Highlight::Type) << pythonType(type) << hl() << " retval = ";

  stream << name << '(';
  {
    CommaSep cs { stream };
    for(const Argument &arg : args) {
      cs << hl(Highlight::Type) << pythonType(arg.type)
         << hl() << ' ' << arg.name;
      if(arg.isOptional())
        stream << " = " << hl(Highlight::Constant) << "None" << hl();
    }
  }
  stream << ')';
}

static void formatHtmlText(std::ostream &stream, std::string_view text)
{
  size_t nextLink { text.find("ImGui_") };
  for(size_t i {}; i < text.size(); ++i) {
    if(i == nextLink) {
      size_t linkStart { i };
      do { ++i; } while(isalpha(text[i]) || text[i] == '_');
      std::string_view linkTo { &text[linkStart], i - linkStart };
      stream << "<a href=\"#" << linkTo << "\">" << linkTo << "</a>";
      nextLink = text.find("ImGui_", i);
    }

    switch(text[i]) {
    case '<':
      stream << "&lt;";
      break;
    case '>':
      stream << "&gt;";
      break;
    case '&':
      stream << "&amp;";
      break;
    case '\0': // if the string ended with a link
      break;
    default:
      stream << text[i];
      break;
    }
  }
}

static void humanBinding(std::ostream &stream)
{
  stream << R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <title>ReaImGui Documentation</title>
  <style>
  body {
    background-color: #080808;
    color: #d9d3d3;
    font-family: monospace;
    font-size: 15px;
    line-height: 20px;
    overflow-anchor: none;
  }
  details { margin-left: 20px; }
  a { text-decoration: none; }
  a, summary { color: #00ff87; }
  a:hover, summary:hover { text-decoration: underline; }
  summary {
    cursor: pointer;
    display: inline-block;
    list-style-type: none;
  }
  summary::before { content: '+ '; margin-left: -20px; }
  details[open] summary::before { content: '- '; }
  summary::-webkit-details-marker { display: none; }
  table { border-collapse: collapse; }
  th {
    padding-left: 0;
    padding-right: 0.5em;
    text-align: left;
    vertical-align: top;
    white-space: nowrap;
  }
  table, pre { white-space: pre-wrap; margin: 0; margin: .3em 0; }
  table + pre { margin-top: 1em; }
  code { border-radius: 3px; color: white; }
  code:hover { text-decoration: underline; cursor: copy; }
  code:active { background-color: #3a3a3a; }
  .st { color: #87afff; /* SkyBlue2 */ }
  .ss { color: #5faf5f; /* DarkSeaGreen4 */ }
  .sc { color: #5f87d7; /* SteelBlue3 */ }
  .sr { color: #d7875f; /* LightSalmon3 */ }
  </style>
</head>
<body>
  <h1>ReaImGui Documentation</h1>
  <p>)" << GENERATED_FOR << "</p>\n\n";

  for(const Function &func : g_funcs) {
    stream << "<details id=\"" << func.name << "\"><summary>";
    stream << (func.isEnum() ? "Constant: " : "Function: ");
    stream << func.name << "</summary>";

    stream << "<table>"
           << "<tr><th>C++</th><td><code>"; func.cppSignature(stream); stream << "</code></td></tr>"
           << "<tr><th>EEL</th><td><code>"; func.eelSignature(stream, false); stream << "</code></td></tr>"
           << "<tr><th>Legacy EEL</th><td><code>";
                             func.eelSignature(stream, true);  stream << "</code></td></tr>"
           << "<tr><th>Lua</th><td><code>"; func.luaSignature(stream); stream << "</code></td></tr>"
           << "<tr><th>Python</th><td><code>"; func.pythonSignature(stream); stream << "</code></td></tr>"
           << "</table>";

    if(!func.doc.empty()) {
      stream << "<pre>";
      formatHtmlText(stream, func.doc);
      stream << "</pre>";
    }

    stream << "</details>";
  }

  stream << R"(<p>EOF</p>

  <script>
  function openTarget()
  {
    var hash = location.hash.substring(1);
    var target = hash && document.getElementById(hash);
    if(target && target.tagName.toLowerCase() === 'details')
      target.open = true;
  }

  window.addEventListener('hashchange', openTarget);
  openTarget();

  document.body.addEventListener('click', function(e) {
    var details = e.target.parentNode;
    if(details && details.tagName.toLowerCase() === 'details')
      history.replaceState(null, '', location.pathname + '#' + details.id);
    else if(e.target.tagName.toLowerCase() === 'code')
      navigator.clipboard.writeText(e.target.textContent);
  });
  </script>
</body>
</html>
)";
}

static const char *pythonCType(const Type &type)
{
  static const std::unordered_map<std::string_view, const char *> ctypes {
    { "void",   "None"      },
    { "bool",   "c_bool"    },
    { "int",    "c_int"     },
    { "double", "c_double"  },
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
    { "bool",   "int"   },
    { "int",    "int"   },
    { "double", "float" },
  };

  return pytypes.at(type);
}

static void pythonBinding(std::ostream &stream)
{
  stream << "# " << GENERATED_FOR << "\n\n"
            "from reaper_python import *\n";

  for(const Function &func : g_funcs) {
    stream << "\ndef " << func.name << '(';
    {
      CommaSep cs { stream };
      for(const Argument &arg : func.args) {
        cs << arg.name;
        if(arg.isOptional() || arg.isOutBuffer(true))
          stream << " = None";
      }
    }
    stream << "):\n"
              "  if not hasattr(" << func.name << ", 'func'):\n"
              "    proc = rpr_getfp('" << func.name << "')\n"
              "    " << func.name << ".func = CFUNCTYPE(";
    {
      CommaSep cs { stream };
      cs << pythonCType(func.type);
      for(const Argument &arg : func.args)
        cs << pythonCType(arg.type);
    }
    stream << ")(proc)\n";

    if(!func.args.empty()) {
      stream << "  args = (";
      CommaSep cs { stream };
      for(const Argument &arg : func.args) {
        bool packed { false };
        if(arg.type.isScalarPtr())
          cs << pythonCType(arg.type.removePtr());
        else if(arg.type.isString())
          cs << (arg.type.isConst() ? "rpr_packsc" : "rpr_packs");
        else if(arg.type.isPointer()) {
          packed = true;
          cs << "rpr_packp('" << arg.type << "', " << arg.name << ')';
        }
        else
          cs << pythonCType(arg.type);

        if(!packed) {
          stream << '(' << arg.name;
          if(arg.isOutBuffer(true))
            stream << " if " << arg.name << " != None else 0";
          stream << ')';
        }
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
      CommaSep cs { stream };
      for(size_t i { 0 }; i < func.args.size(); ++i) {
        const Argument &arg { func.args[i] };
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

    if(!func.type.isVoid() || func.hasOutBuffers()) {
      stream << "  return ";
      CommaSep cs { stream };
      if(!func.type.isVoid()) {
        if(func.type.isString())
          cs << "str(rval.decode())";
        else if(func.type.isPointer())
          cs << "rpr_unpackp('" << func.type << "', rval)";
        else
          cs << "rval";
      }
      for(size_t i { 0 }; i < func.args.size(); ++i) {
        const Argument &arg { func.args[i] };
        if(!arg.isOutBuffer())
          continue;
        if(arg.type.isScalarPtr())
          cs << pythonScalarType(arg.type.removePtr())
            << "(args[" << i << "" << "].value)";
        else if(arg.type.isString() && !arg.type.isConst())
          cs << "rpr_unpacks(" << "args[" << i << "])";
        else
          cs << arg.name;
        if(arg.isOptional())
          stream << " if " << arg.name << " != None else None";
      }
      stream << "\n";
    }
  }
}

static int plugin_register(const char *key, void *value)
{
  if(strstr(key, "APIdef_") == key)
    addFunc(key + strlen("APIdef_"), reinterpret_cast<const char *>(value));
  return 0;
}

static int fakeFunc()
{
  return 0;
}

static void *getFunc(const char *name)
{
  if(!strcmp(name, "plugin_register"))
    return reinterpret_cast<void *>(&plugin_register);

  return reinterpret_cast<void *>(&fakeFunc);
}

int main(int argc, const char *argv[])
{
  reaper_plugin_info_t rec { REAPER_PLUGIN_VERSION };
  rec.GetFunc = getFunc;

  REAPER_PLUGIN_ENTRYPOINT(nullptr, &rec);

  std::sort(g_funcs.begin(), g_funcs.end());

  const std::string_view lang { argc >= 2 ? argv[1] : "cpp" };

  if(lang == "cpp")
    cppBinding(std::cout);
  else if(lang == "human")
    humanBinding(std::cout);
  else if(lang == "python")
    pythonBinding(std::cout);
  else {
    fprintf(stderr, "don't know how to generate a binding for '%s'\n", lang.data());
    return 1;
  }

  return 0;
}
