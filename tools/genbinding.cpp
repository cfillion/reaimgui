#include "version.hpp"

#include <algorithm>
#include <iostream>
#include <string_view>
#include <vector>

#include <reaper_plugin.h>

extern "C" int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE, reaper_plugin_info_t *);

struct Argument {
  std::string_view type;
  std::string_view name;
};

struct Function {
  std::string_view name;
  std::string_view type;
  std::vector<Argument> args;

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

static void cppBinding(std::ostream &stream)
{
  stream << "// Generated for ReaImGui v" << REAIMGUI_VERSION << R"(

#ifndef REAPER_IMGUI_FUNCTIONS_H
#define REAPER_IMGUI_FUNCTIONS_H

#include <reaper_plugin_functions.h>

class ImGui_Context;
class ImGui_DrawList;
class ImGui_ListClipper;

struct reaper_array;

template<typename T>
class ReaImGuiFunc;

template<typename R, typename... Args>
class ReaImGuiFunc<R(Args...)>
{
public:
  ReaImGuiFunc(const char *name) : m_name { name }, m_proc { nullptr } {}
  operator bool() const { return proc() != nullptr; }
  auto operator()(Args... args) { return proc()(std::forward<Args>(args)...); }

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

    if(func.type == "int" && func.args.empty())
      stream << "ReaImGuiEnum ";
    else {
      stream << "ReaImGuiFunc<" << func.type << '(';
      bool sep { false };
      for(const Argument &arg : func.args) {
        if(sep)
          stream << ", ";
        else
          sep = true;

        stream << arg.type << ' ' << arg.name;
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
  else
    fprintf(stderr, "don't know how to generate a binding for '%s'\n", lang.data());
}
