/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#define IMPORT_GENBINDINGS_API
#include "../src/api.hpp"
#include "version.hpp"

#include <algorithm>
#include <cstring>
#include <deque>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

constexpr const char *GENERATED_FOR { "Generated for ReaImGui v" REAIMGUI_VERSION };

struct Type {
  Type(const char *val)             : m_value { val } {}
  Type(const std::string_view &val) : m_value { val } {}

  bool isVoid()      const { return m_value == "void"; }
  bool isInt()       const { return m_value == "int"; }
  bool isBool()      const { return m_value == "bool"; }
  bool isDouble()    const { return m_value == "double"; }
  bool isString()    const { return m_value == "const char*" || m_value == "char*"; }
  bool isPointer()   const { return m_value.size() >= 1 && m_value.back() == '*'; }
  bool isConst()     const { return m_value.find("const ") == 0; }
  bool isNumber()    const { return isInt() || isDouble(); };
  bool isScalar()    const { return isBool() || isNumber(); }
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

  bool isInput() const
    { return !isOutput() || name.find("In") != std::string_view::npos; }
  bool isOutput() const
    { return name.find("Out") != std::string_view::npos; }
  bool isOptional() const
    { return name.find("Optional") != std::string_view::npos; }
  bool isBigBuf() const
    { return name.find("NeedBig") != std::string_view::npos; }
  bool isBufSize() const
    { return name.find("_sz") == name.size() - 3; }
  std::string_view bufName() const { return name.substr(0, name.size() - 3); }

  std::string_view humanName() const;
};

struct Function {
  const API::Section *section;
  std::string_view name;
  Type type;
  API::LineRange lines;
  std::vector<Argument> args;
  std::string_view doc;
  std::string_view displayName;
  std::deque<const API::Section *> sections;

  bool operator<(const Function &) const;

  bool isEnum() const { return type.isInt() && args.empty(); }
  bool hasOutputArgs() const;
  bool hasOptionalArgs() const;

  void cppSignature(std::ostream &) const;
  void luaSignature(std::ostream &) const;
  void eelSignature(std::ostream &, bool legacySyntax = false) const;
  void pythonSignature(std::ostream &) const;
};

bool Function::operator<(const Function &o) const
{
  const size_t maxDepth { std::min(sections.size(), o.sections.size()) };
  for(size_t i {}; i < maxDepth; ++i) {
    if(sections[i] != o.sections[i])
      return strcmp(sections[i]->title, o.sections[i]->title) < 0;
  }

  if(sections.size() != o.sections.size())
    return sections.size() < o.sections.size();

  return displayName < o.displayName;
}

static std::deque<Function> g_funcs;

static const char *nextString(const char *&str)
{
  return str += strlen(str) + 1;
}

static void addFunc(const API *api)
{
  const char *def { api->definition() };
  Function func { api->m_section, api->name(), def, api->m_lines };

  std::string_view argTypes { nextString(def) };
  std::string_view argNames { nextString(def) };
  func.doc = nextString(def);

  // C++20's starts_with isn't available when building for old macOS
  func.displayName = func.name.substr(0, strlen("ImGui_")) == "ImGui_"
                   ? func.name.substr(strlen("ImGui_")) : func.name;

  while(argTypes.size() > 0 && argNames.size() > 0) {
    const size_t typeLen { argTypes.find(',') },
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

  const API::Section *section { func.section };
  do { func.sections.push_front(section); } while((section = section->parent));

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
class ImGui_DrawListSplitter;
class ImGui_Font;
class ImGui_ListClipper;
class ImGui_TextFilter;
class ImGui_Viewport;

struct reaper_array;

template<typename T>
class ReaImGuiFunc;

template<typename R, typename... Args>
class ReaImGuiFunc<R(Args...)>
{
public:
  ReaImGuiFunc(const char *name) : m_name { name }, m_proc { nullptr } {}
  operator bool() { return proc() != nullptr; }
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

#endif
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
    tag += "sc";
    break;
  case Highlight::End:
    return "Invalid Highlight";
  }
  tag += "\">";
  return tag;
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
      if(arg.isOutput() && !arg.isBufSize()) {
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
    const bool listOutputs { hasOptionalArgs() };
    CommaSep cs { stream };
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
      if(arg.isOptional())
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
  if(hasOutputArgs()) {
    CommaSep cs { stream };
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

  stream << name << '(';
  {
    CommaSep cs { stream };
    for(const Argument &arg : args) {
      if(arg.isBufSize() || !arg.isInput())
        continue;
      cs << hl(Highlight::Type) << pythonType(arg.type)
         << hl() << ' ' << arg.name;
      if(arg.isOptional())
        stream << " = " << hl(Highlight::Constant) << "None" << hl();
    }
  }
  stream << ')';
}

static auto findNewSection
  (const Function &func, std::vector<const API::Section *> &oldSections)
{
  auto oldSection { oldSections.begin() };
  auto newSection { func.sections.begin() };
  while(oldSection != oldSections.end() && newSection != func.sections.end() &&
        *oldSection == *newSection)
    ++oldSection, ++newSection;
  oldSections.erase(oldSection, oldSections.end());
  std::copy(newSection, func.sections.end(), std::back_inserter(oldSections));
  return newSection;
}

static void formatHtmlText(std::ostream &stream, std::string_view text)
{
  // outputting char by char is slower than as many as possible at once
  while(!text.empty()) {
    const size_t nextEntity { text.find_first_of("<>&") };
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

struct Token {
  using List = std::vector<std::unique_ptr<Token>>;
  using Iterator = List::iterator;

  Token(const std::string_view &range) : m_range { range } {}
  virtual ~Token() = default;

  virtual void format(std::ostream &stream,
    Iterator &nextToken, const Iterator &end) const
  {
    std::string_view text { m_range };
    while(nextToken != end && contains(**nextToken)) {
      formatHtmlText(stream, (*nextToken)->before(text));
      (*nextToken)->advance(text);
      (*nextToken)->format(stream, ++nextToken, end);
    }
    formatHtmlText(stream, text);
  }

  std::string_view before(const std::string_view &text) const
  {
    return text.substr(0, m_range.begin() - text.begin());
  }

  void advance(std::string_view &text) const
  {
    text.remove_prefix(m_range.end() - text.begin());
  }

  bool contains(const Token &o) const
  {
    return o.m_range.begin() < m_range.end();
  }

  std::string_view m_range;
};

static bool operator<(const std::unique_ptr<Token> &a,
                      const std::unique_ptr<Token> &b)
{
  const auto cmp { a->m_range.begin() - b->m_range.begin() };
  if(cmp == 0) // longest matches first
    return &a->m_range.back() > &b->m_range.back();
  else
    return cmp < 0;
}

static void eachLine(std::string_view text,
  const std::function<void (const std::string_view &)> &callback)
{
  while(!text.empty()) {
    size_t nl { text.find("\n", 1) };
    if(nl != std::string_view::npos)
      ++nl; // include the newline in the new line
    callback(text.substr(0, nl));
    if(nl >= text.size())
      return;
    text.remove_prefix(nl);
  }
}

static void eachBlock(std::string_view text,
  const std::function<bool (const std::string_view &, bool inBlock)> &addLine,
  const std::function<void (const std::string_view &)> &addBlock)
{
  const char *begin {};
  eachLine(text, [&](const std::string_view &line) {
    if(addLine(line, begin != nullptr)) {
      if(!begin)
        begin = line.begin();
    }
    else if(begin) {
      addBlock({ begin, static_cast<size_t>(line.begin() - begin) });
      begin = nullptr;
    }
  });
  if(begin) // when the block does not end before the text
    addBlock({ begin, static_cast<size_t>(text.end() - begin) });
}

struct CodeBlock : Token {
  static constexpr std::string_view s_prefix { "\x20\x20" };

  static void tokenize(const std::string_view &text, List &tokens)
  {
    eachBlock(text, [&](std::string_view line, bool inBlock) {
      if(line.size() >= s_prefix.size())
        inBlock = line.substr(0, s_prefix.size()) == s_prefix;
      if(inBlock) {
        if(line.size() >= s_prefix.size())
          line.remove_prefix(s_prefix.size());
        tokens.push_back(std::make_unique<Token>(line));
      }
      return inBlock;
    }, [&](const std::string_view &block) {
      tokens.push_back(std::make_unique<CodeBlock>(block));
    });
  }

  CodeBlock(const std::string_view &range) : Token { range } {}

  void format(std::ostream &stream,
    Iterator &nextToken, const Iterator &end) const override
  {
    std::string_view text { m_range };
    stream << "<code>";
    // only print line tokens (and subtokens) to remove s_prefix
    while(nextToken != end && contains(**nextToken)) {
      (*nextToken)->advance(text);
      (*nextToken)->format(stream, ++nextToken, end);
    }
    stream << "</code>";
  }
};

struct Reference : Token {
  static void tokenize(const std::string_view &text, List &tokens)
  {
    const std::string_view prefix { "ImGui_" };
    for(const Function &func : g_funcs) {
      size_t pos {};
      do {
        size_t size { func.displayName.size() };
        pos = text.find(func.displayName, pos);
        if(pos == std::string_view::npos)
          break;
        if(pos > prefix.size() &&
           text.substr(pos - prefix.size(), prefix.size()) == prefix)
          pos -= prefix.size(), size += prefix.size();
        // check if match is not part of a bigger word
        const size_t end { pos + size };
        if((pos == 0 || !isalpha(text[pos - 1])) &&
            (end >= text.size() || !isalpha(text[end])))
          tokens.push_back(std::make_unique<Reference>(&func, text.substr(pos, size)));
        pos += size;
      } while(true);
    }
  }

  Reference(const Function *func, const std::string_view &range)
    : Token { range }, m_func { func } {}

  void format(std::ostream &stream,
    Iterator &nextToken, const Iterator &end) const override
  {
    stream << "<a href=\"#" << m_func->displayName << "\">";
    formatHtmlText(stream, m_range);
    stream << "</a>";

    while(nextToken != end && contains(**nextToken)) ++nextToken;
  }

  const Function *m_func;
};

static void formatHtmlBlock(std::ostream &stream, std::string_view text)
{
  std::vector<std::unique_ptr<Token>> tokens;
  CodeBlock::tokenize(text, tokens);
  Reference::tokenize(text, tokens);
  std::sort(tokens.begin(), tokens.end());

  auto token { tokens.begin() };
  stream << "<pre>";
  while(token != tokens.end()) {
    formatHtmlText(stream, (*token)->before(text));
    (*token)->advance(text);
    (*token)->format(stream, ++token, tokens.end());
  }
  formatHtmlText(stream, text);
  stream << "</pre>";
}

static void formatHtmlSlug(std::ostream &stream, const std::string_view &text)
{
  bool prevWasPrintable { false };
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

static void humanBinding(std::ostream &stream)
{
  stream << R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <title>ReaImGui Documentation</title>
  <style>
  html { scroll-padding-top: 1em; }
  body {
    background-color: #080808;
    color: #d9d3d3;
    font-size: 15px;
    line-height: 20px;
    margin: 0;
    overflow-anchor: none;
  }
  body, pre, code { font-family: Consolas, monospace; }
  aside, pre code { background-color: #262626; /* Grey15 */ }
  aside {
    border-right: 1px solid #6f6f6f;
    overflow-y: auto;
    position: fixed;
    top: 0;
    bottom: 0;
    width: 200px;
  }
  aside ol {
    list-style-type: none;
    padding-left: 0;
  }
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
  h2, h3, h4, h5, h6, pre code { margin: 1em 0 1rem 0; }
  h1 { font-size: 2.3em; }
  h2 { font-size: 1.8em; }
  h3 { font-size: 1.4em; }
  h4 { font-size: 1.2em; }
  h5 { font-size: 1.0em; }
  h6 { font-size: 0.9em; }
  h2:before { content: '〉'; color: #6f6f6f; }
  aside p, aside li a, main { padding-left: 1em; }
  details { margin-left: 20px; }
  a { text-decoration: none; }
  a, summary { color: #00ff87; /* SpringGreen1 */ }
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
  table, pre { white-space: pre-wrap; }
  code, code a { color: white; }
  table code:hover { text-decoration: underline; cursor: copy; }
  table code:active, aside a:hover { background-color: #3a3a3a; }
  pre code {
    border-radius: 5px;
    display: inline-block;
    padding: 5px;
  }
  .st { color: #87afff; /* SkyBlue2 */ }
  .ss { color: #5faf5f; /* DarkSeaGreen4 */ }
  .sc { color: #5f87d7; /* SteelBlue3 */ }
  .sr { color: #d7875f; /* LightSalmon3 */ }
  .source a { color: gray; }
  </style>
</head>
<body>
  <aside>
    <p><strong>Table of Contents</strong></p>
    <ol>)";

  const API::Section *section {};
  for(const Function &func : g_funcs) {
    if(func.sections[0] == section)
      continue;

    section = func.sections[0];

    stream << "<li><a href=\"#";
    formatHtmlSlug(stream, section->title); stream << "\">";
    formatHtmlText(stream, section->title);
    stream << "</a></li>";
  }

  stream << R"(
    </ol>
  </aside>
  <main>
    <h1>ReaImGui Documentation</h1>
    <p>)" << GENERATED_FOR << "</p>\n\n";

  std::vector<const API::Section *> sections;
  for(const Function &func : g_funcs) {
    for(auto it { findNewSection(func, sections) };
        it != func.sections.end(); ++it) {
      const API::Section *section { *it };
      const auto level { std::distance(func.sections.begin(), it) + 2 };

      stream << "<h" << level << " id=\"";
      for(auto slugIt { func.sections.begin() }; slugIt <= it; ++slugIt) {
        if(slugIt != func.sections.begin())
          stream << '_';
        formatHtmlSlug(stream, (*slugIt)->title);
      }
      stream << "\">";
      formatHtmlText(stream, section->title);
      stream << "</h" << level << '>';

      if(section->help)
        formatHtmlBlock(stream, section->help);
    }
    
    stream << "<details id=\"" << func.displayName << "\"><summary>";
    stream << (func.isEnum() ? "Constant: " : "Function: ");
    stream << func.displayName << "</summary>";

    stream << "<table>"
           << "<tr><th>C++</th><td><code>";        func.cppSignature(stream);        stream << "</code></td></tr>"
           << "<tr><th>EEL</th><td><code>";        func.eelSignature(stream, false); stream << "</code></td></tr>"
           << "<tr><th>Legacy EEL</th><td><code>"; func.eelSignature(stream, true);  stream << "</code></td></tr>"
           << "<tr><th>Lua</th><td><code>";        func.luaSignature(stream);        stream << "</code></td></tr>"
           << "<tr><th>Python</th><td><code>";     func.pythonSignature(stream);     stream << "</code></td></tr>"
           << "</table>";

    if(!func.doc.empty())
      formatHtmlBlock(stream, func.doc);

    stream << "<p class=\"source\">"
              "<a href=\"https://github.com/cfillion/reaimgui/blob/v"
              REAIMGUI_VERSION "/api/" << func.section->file << ".cpp#L"
           << func.lines.first << "-L" << func.lines.second
           << "\">View source</a></p>";

    stream << "</details>";
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
</body>
</html>
)";
}

static const char *pythonCType(const Type &type)
{
  static const std::unordered_map<std::string_view, const char *> ctypes {
    { "void",   "None"     },
    { "bool",   "c_bool"   },
    { "int",    "c_int"    },
    { "double", "c_double" },
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
        if(arg.isBufSize() || !arg.isInput())
          continue;
        cs << arg.name;
        if(arg.isOptional())
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

    if(!func.type.isVoid() || func.hasOutputArgs()) {
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
      stream << "\n";
    }
  }
}

int main(int argc, const char *argv[])
{
  while(const API *func { API::enumAPI() }) {
    if(func->definition()) // only handle function exported to ReaScript
      addFunc(func);
  }
  std::sort(g_funcs.begin(), g_funcs.end());

  const std::string_view lang { argc >= 2 ? argv[1] : "cpp" };

  if(lang == "cpp")
    cppBinding(std::cout);
  else if(lang == "human")
    humanBinding(std::cout);
  else if(lang == "python")
    pythonBinding(std::cout);
  else {
    std::cerr << "don't know how to generate a binding for '"
              << lang << "'" << std::endl;
    return 1;
  }

  return 0;
}
