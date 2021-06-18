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
  bool isPointer() const { return m_value.size() >= 1 && m_value.back() == '*'; }
  bool isString() const { return m_value == "const char*" || m_value == "char*"; }
  bool isConst() const { return m_value.find("const ") == 0; }
  bool isScalar() const {
    return m_value == "bool" || m_value == "int" || m_value == "double";
  }
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

  bool isOutBuffer() const {
    return name.find("In")  == std::string_view::npos &&
           name.find("Out") != std::string_view::npos;
  }

  bool isOptional() const {
    return name.find("Optional") != std::string_view::npos;
  }
};

struct Function {
  std::string_view name;
  Type type;
  std::vector<Argument> args;

  bool isEnum() const { return type.isInt() && args.empty(); }
  bool operator<(const Function &o) const { return name < o.name; }
};

static std::vector<Function> g_funcs;

static void addFunc(const char *name, const char *def)
{
  Function func { name, def };

  std::string_view argTypes { def += strlen(def) + 1 };
  std::string_view argNames { def += strlen(def) + 1 };

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
        if(arg.isOptional() || arg.isOutBuffer())
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

    bool retAllArgs { false };
    if(!func.args.empty()) {
      stream << "  args = (";
      CommaSep cs { stream };
      for(const Argument &arg : func.args) {
        bool packed { false };
        if(arg.type.isScalarPtr()) {
          retAllArgs = true;
          cs << pythonCType(arg.type.removePtr());
        }
        else if(arg.type.isString()) {
          retAllArgs = true;
          cs << (arg.type.isConst() ? "rpr_packsc" : "rpr_packs");
        }
        else if(arg.type.isPointer()) {
          packed = true;
          cs << "rpr_packp('" << arg.type << "', " << arg.name << ')';
        }
        else
          cs << pythonCType(arg.type);

        if(!packed) {
          stream << '(' << arg.name;
          if(arg.isOutBuffer())
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

    if(!func.type.isVoid() || !func.args.empty()) {
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
      if(retAllArgs) {
        for(size_t i { 0 }; i < func.args.size(); ++i) {
          const Argument &arg { func.args[i] };
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
  else if(lang == "python")
    pythonBinding(std::cout);
  else {
    fprintf(stderr, "don't know how to generate a binding for '%s'\n", lang.data());
    return 1;
  }

  return 0;
}
