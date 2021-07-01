#include <clang/AST/Attr.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

struct ListPrinter {
  ListPrinter(llvm::raw_ostream &stream, const std::string &glue = ", ")
    : m_stream { stream }, m_first { true }, m_glue { glue } {}

  void next()
  {
    if(m_first)
      m_first = false;
    else
      m_stream << m_glue;
  }

  template<typename T>
  llvm::raw_ostream &operator<<(const T rhs)
  {
    next();
    m_stream << rhs;
    return m_stream;
  }

private:
  llvm::raw_ostream &m_stream;
  bool m_first;
  std::string m_glue;
};

class Function {
public:
  Function(const clang::FunctionDecl *);
  bool isInMainFile() const;
  bool isApiExport() const;
  std::string helpText() const;

  void printExportDecl(llvm::raw_ostream &) const;

private:
  bool getAnnotation(const char *key, std::string *value = nullptr) const;
  std::string makeReaperDef() const;

  void printType(llvm::raw_ostream &, const clang::QualType &) const;
  void printParameters(llvm::raw_ostream &,
    bool types, bool names, const std::string &glue = ", ") const;
  void printCatch(llvm::raw_ostream &, const char *execptionType) const;
  void printStringLiteral(llvm::raw_ostream &, const std::string &) const;

  const clang::FunctionDecl *m_decl;
  const clang::ASTContext &m_ctx;
  const clang::SourceManager &m_sm;
};

Function::Function(const clang::FunctionDecl *decl)
  : m_decl { decl }, m_ctx { decl->getASTContext() },
    m_sm { m_ctx.getSourceManager() }
{
}

bool Function::isInMainFile() const
{
  return m_sm.isInMainFile(m_sm.getExpansionLoc(m_decl->getLocation()));
}

bool Function::isApiExport() const
{
  return getAnnotation("reaimgui_api");
}

bool Function::getAnnotation(const char *key, std::string *value) const
{
  const auto &annotations { m_decl->specific_attrs<clang::AnnotateAttr>() };
  for(const clang::AnnotateAttr *attr : annotations) {
    const std::string &text { attr->getAnnotation() };
    const size_t sep { text.find('=') };

    if(!text.compare(0, sep, key)) {
      if(value && sep != std::string::npos)
        *value = text.substr(sep + 1);
      return true;
    }
  }
  return false;
}

std::string Function::helpText() const
{
  if(const clang::RawComment *c { m_ctx.getRawCommentForAnyRedecl(m_decl) })
    return c->getFormattedText(m_sm, m_ctx.getDiagnostics());

  std::string doc;
  getAnnotation("reaimgui_doc", &doc);
  return doc;
}

void Function::printType(llvm::raw_ostream &out, const clang::QualType &type) const
{
  std::string str { type.getAsString(m_ctx.getPrintingPolicy()) };
  const size_t pos { str.find(" *") };
  if(pos != std::string::npos)
    str.erase(pos, 1);
  out << str;
}

void Function::printParameters(llvm::raw_ostream &out,
  bool types, bool names, const std::string &glue) const
{
  ListPrinter lp { out, glue };
  for(const clang::ParmVarDecl *param : m_decl->parameters()) {
    lp.next();

    if(types)
      printType(out, param->getType());
    if(types && names)
      out << ' ';
    if(names) {
      if(const clang::IdentifierInfo *id { param->getIdentifier() })
        out << id->getName();
      else
        out << "UNNAMED";
    }
  }
}

void Function::printExportDecl(llvm::raw_ostream &out) const
{
  out << "\n"
         "// " << m_decl->getDeclName() << " from ";
  m_decl->getBeginLoc().print(out, m_sm);
  out << '\n';

  out << "extern ";
  printType(out, m_decl->getReturnType());
  out << ' ' << m_decl->getDeclName() << '(';
  printParameters(out, true, false);
  out << ");\n";

  printType(out, m_decl->getReturnType());
  out << " APIcall_" << m_decl->getDeclName() << '(';
  printParameters(out, true, true);
  out << ") noexcept\n"
         "try {\n"
         "  return " << m_decl->getDeclName() << '(';
  printParameters(out, false, true);
  out << ");\n"
         "}\n";
  printCatch(out, "reascript_error");
  printCatch(out, "imgui_error");

  out << "static const API APIdecl_" << m_decl->getDeclName() << " {\n"
         "  ";
  printStringLiteral(out, m_decl->getDeclName().getAsString());
  out << ",\n"
         "  reinterpret_cast<void *>(&APIcall_" << m_decl->getDeclName() << "),\n"
         "  reinterpret_cast<void *>(&InvokeReaScriptAPI<&APIcall_"
      <<      m_decl->getDeclName() << ">),\n"
         "  reinterpret_cast<void *>(const_cast<char *>(\n"
         "    ";
  printStringLiteral(out, makeReaperDef());
  out << "\n"
         "  ))\n"
         "};\n";
}

void Function::printCatch(llvm::raw_ostream &out, const char *exceptionType) const
{
  out << "catch(const " << exceptionType << " &e) {\n"
         "  API::handleError(";
  printStringLiteral(out, m_decl->getDeclName().getAsString());
  out << ", e);\n"
         "  return static_cast<";
  printType(out, m_decl->getReturnType());
  out << ">(0);\n"
         "}\n";
}

void Function::printStringLiteral(llvm::raw_ostream &out, const std::string &str) const
{
  clang::StringLiteral::Create(m_ctx, str, clang::StringLiteral::Ascii, false, {}, {})
    ->outputString(out);
};

std::string Function::makeReaperDef() const
{
  const clang::PresumedLoc &startLoc { m_sm.getPresumedLoc(m_decl->getBeginLoc()) },
                           &endLoc   { m_sm.getPresumedLoc(m_decl->getEndLoc()) };

  std::string str;
  llvm::raw_string_ostream out { str };
  ListPrinter lp { out, { "\0", 1 } };
  lp.next(); printType(out, m_decl->getReturnType());
  lp.next(); printParameters(out, true, false, ",");
  lp.next(); printParameters(out, false, true, ",");
  lp << helpText();
  lp << "group";
  lp << startLoc.getLine();
  lp << endLoc.getLine();
  return out.str();
}

class ASTConsumer : public clang::ASTConsumer {
public:
  bool HandleTopLevelDecl(clang::DeclGroupRef group) override
  {
    for(clang::Decl *decl : group) {
      if(auto funcDecl { dynamic_cast<clang::FunctionDecl *>(decl) }) {
        Function func { funcDecl };
        if(func.isInMainFile() && func.isApiExport() && funcDecl->isThisDeclarationADefinition())
          func.printExportDecl(llvm::outs());
      }
    }
    return true;
  }
};

class GenApiDefAction : public clang::ASTFrontendAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, clang::StringRef) override
  {
    llvm::outs() << R"(// DO NOT EDIT THIS FILE BY HAND

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
class reaper_array;
)";

    // ci.getPreprocessor().SetSuppressIncludeNotFoundError(true);
    ci.getDiagnostics().setSuppressAllDiagnostics(true);
    return std::make_unique<ASTConsumer>();
  }
};

int main(int argc, const char *argv[])
{
  std::string error;
  auto compilationDatabase
    { clang::tooling::FixedCompilationDatabase::loadFromCommandLine
      (argc, argv, error) };
  if(!compilationDatabase || argc < 2) {
    // '--' was not found in argv or there was no source_file
    llvm::errs() << "Usage: genapidef source_file -- clang_options...\n";
    return 1;
  }

  auto frontend { clang::tooling::newFrontendActionFactory<GenApiDefAction>() };

  clang::tooling::ClangTool tool { *compilationDatabase, { argv[1] } };
  return tool.run(frontend.get());
}
