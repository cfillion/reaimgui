// cmake --build build --target genapidef && build/tools/genapidef/genapidef api/button.cpp -Isrc -std=c++17 -Ivendor -Ivendor/WDL -Ivendor/WDL/WDL -I/usr/local/Cellar/boost/1.76.0/include

#include <clang-c/Index.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>
#include <sys/types.h>

struct Argument {
  std::string type, name;
};

struct Function {
  bool isAPI;
  std::string type, name, help, file;
  unsigned int firstLine, lastLine;
  std::vector<Argument> args;

  void printExport(std::ostream &) const;
  void printParameters(std::ostream &,
    bool types, bool names, const std::string &glue = ", ") const;
  void printCatch(std::ostream &, const char *exceptionType) const;
};

struct UnitData {
  std::vector<Function> funcs;
};

static void getLineRange(CXCursor cursor, unsigned int *begin, unsigned int *end)
{
  CXSourceRange sourceRange { clang_getCursorExtent(cursor) };
  CXSourceLocation beginLoc { clang_getRangeStart(sourceRange) },
                   endLoc   { clang_getRangeEnd(sourceRange) };
  clang_getPresumedLocation(beginLoc, nullptr, begin, nullptr);
  clang_getPresumedLocation(endLoc,   nullptr, end, nullptr);

  CXSourceRange comment { clang_Cursor_getCommentRange(cursor) };
  if(!clang_Range_isNull(comment)) {
    CXSourceLocation commentLoc { clang_getRangeStart(comment) };
    unsigned int commentLine;
    clang_getPresumedLocation(commentLoc, nullptr, &commentLine, nullptr);

    if(commentLine && commentLine < *begin)
      *begin = commentLine;
  }
}

static std::string getString(CXString str)
{
  const char *cstr { clang_getCString(str) };
  std::string out { cstr ? cstr : "" };
  clang_disposeString(str);
  return out;
}

static std::string getTypeString(CXType type)
{
  std::string out { getString(clang_getTypeSpelling(type)) };

  const size_t pos { out.find(" *") };
  if(pos != std::string::npos)
    out.erase(pos, 1);

  return out;
}

static void visitFuncAttr(CXCursor cursor, Function *func)
{
  CXString spelling { clang_getCursorSpelling(cursor) };
  if(!strcmp(clang_getCString(spelling), "reaimgui_api"))
    func->isAPI = true;
  clang_disposeString(spelling);
}

static CXChildVisitResult visitFunctionChild(CXCursor cursor, CXCursor, CXClientData data)
{
  Function *func { static_cast<Function *>(data) };

  switch(clang_getCursorKind(cursor)) {
  case CXCursorKind::CXCursor_AnnotateAttr:
    visitFuncAttr(cursor, func);
    break;
  default:
    break;
  }

  return CXChildVisit_Continue;
}

static void visitFunction(CXCursor cursor, UnitData *ud)
{
  Function func{};
  clang_visitChildren(cursor, &visitFunctionChild, &func);

  if(!func.isAPI)
    return;

  getLineRange(cursor, &func.firstLine, &func.lastLine);

  const CXType type { clang_getCursorType(cursor) };
  func.type = getTypeString(clang_getResultType(type));
  func.name = getString(clang_getCursorSpelling(cursor));
  func.help = getString(clang_Cursor_getRawCommentText(cursor));

  const int argc { clang_Cursor_getNumArguments(cursor) };
  for(int i {}; i < argc; ++i) {
    CXCursor argCursor { clang_Cursor_getArgument(cursor, i) };

    func.args.push_back({
      getTypeString(clang_getArgType(type, i)),
      getString(clang_getCursorSpelling(argCursor)),
    });
  }

  ud->funcs.push_back(func);
}

static CXChildVisitResult visitRoot(CXCursor cursor, CXCursor, CXClientData data)
{
  if(!clang_Location_isFromMainFile(clang_getCursorLocation(cursor)))
    return CXChildVisit_Continue;

  UnitData *unitData { static_cast<UnitData *>(data) };

  switch(clang_getCursorKind(cursor)) {
  case CXCursorKind::CXCursor_FunctionDecl:
    visitFunction(cursor, unitData);
    break;
  default:
    break;
  }

  return CXChildVisit_Continue;
}

static int exec(const std::vector<char *> &args, std::string &output)
{
  int fd[2];
  if(pipe(fd)) {
    perror("pipe() failed");
    return -1;
  }

  const pid_t pid { fork() };

  if(pid == -1) {
    perror("fork() failed");
    return -1;
  }
  else if(pid == 0) {
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    execvp(args[0], args.data());
    perror("execvp() failed");
    exit(1);
  }

  close(fd[1]);

  char buf[1024];
  ssize_t size;
  while((size = read(fd[0], buf, std::size(buf))) && size >= 0)
    output.append(buf, size);
  close(fd[0]);

  if(size < 0) {
    perror("read() failed");
    return -1;
  }

  int status;
  wait(&status);

  return WEXITSTATUS(status);
}

static int preprocess(int argc, char *argv[], std::string &source)
{
  std::vector<char *> args { argv, argv + argc };
  args[0] = const_cast<char *>("clang++");
  args.push_back(const_cast<char *>("-E"));
  args.push_back(const_cast<char *>("-C"));
  args.push_back(nullptr);
  return exec(args, source);
}

void Function::printParameters(std::ostream &out,
  bool types, bool names, const std::string &glue) const
{
  // ListPrinter lp { out, glue };
  for(const Argument &arg : args) {
    // lp.next();

    if(types)
      out << arg.type;
    if(types && names)
      out << ' ';
    if(names) {
      if(arg.name.empty())
        out << "UNNAMED";
      else
        out << arg.name;
    }
  }
}

void Function::printExport(std::ostream &out) const
{
  out << "\n"
         "// " << name << " at " << file << ':' << firstLine << '\n';

  out << "extern " << type;
  out << ' ' << name << '(';
  printParameters(out, true, false);
  out << ");\n";

  out << type << " APIcall_" << name << '(';
  printParameters(out, true, true);
  out << ") noexcept\n"
         "try {\n"
         "  return " << name << '(';
  printParameters(out, false, true);
  out << ");\n"
         "}\n";
  printCatch(out, "reascript_error");
  printCatch(out, "imgui_error");

  out << "static const API APIdecl_" << name << " {\n"
         "  ";
  // printStringLiteral(out, m_decl->getDeclName().getAsString());
  out << ",\n"
         "  reinterpret_cast<void *>(&APIcall_" << name << "),\n"
         "  reinterpret_cast<void *>(&InvokeReaScriptAPI<&APIcall_"
      <<      name << ">),\n"
         "  reinterpret_cast<void *>(const_cast<char *>(\n"
         "    ";
  // printStringLiteral(out, makeReaperDef());
  out << "\n"
         "  ))\n"
         "};\n";
}

void Function::printCatch(std::ostream &out, const char *exceptionType) const
{
  out << "catch(const " << exceptionType << " &e) {\n"
         "  API::handleError(";
  // printStringLiteral(out, m_decl->getDeclName().getAsString());
  out << ", e);\n"
         "  return static_cast<" << type << ">(0);\n"
         "}\n";
}

int main(int argc, char *argv[])
{
  const char *fn { argv[1] };

  // libclang does not support expanding proprecessor macros at this time
  std::string source;
  const int status { preprocess(argc, argv, source) };
  if(status) {
    fprintf(stderr, "failed to preprocess %s\n", fn);
    return status;
  }

  CXUnsavedFile files[] {
    { fn, source.c_str(), source.size() },
  };

  constexpr int flags {
    CXTranslationUnit_KeepGoing |
    CXTranslationUnit_SingleFileParse
  };

  CXIndex index { clang_createIndex(1, 0) };
  CXTranslationUnit unit
    { clang_parseTranslationUnit(index, fn,
        &argv[2], argc - 2, files, std::size(files), flags) };

  if(!unit) {
    fprintf(stderr, "failed to parse %s\n", fn);
    return 1;
  }

  UnitData data;
  CXCursor root { clang_getTranslationUnitCursor(unit) };
  clang_visitChildren(root, visitRoot, &data);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  std::cout << R"(// DO NOT EDIT THIS FILE BY HAND

#include "api.hpp"
#include "api_vararg.hpp"

class Context;
using ImGui_Context = Context;
class ImGui_DrawfList;
class Font;
using ImGui_Font = Font;
class Viewport;
using ImGui_Viewport = Viewport;
class ImGui_ListClipper;
struct reaper_array;
)";

  for(const Function &func : data.funcs)
    func.printExport(std::cout);

  return 0;
}
