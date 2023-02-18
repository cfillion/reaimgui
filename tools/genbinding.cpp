/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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
#include <cmark.h>
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
  Function(const API *);

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

Function::Function(const API *api)
  : section { api->m_section }, name { api->name() },
    type { api->definition() }, lines { api->m_lines }
{
  const char *def { api->definition() };
  std::string_view argTypes { nextString(def) };
  std::string_view argNames { nextString(def) };
  doc = nextString(def);
  std::string_view argDefvs { nextString(def) }; // non-standard field

  // C++20's starts_with isn't available when building for old macOS
  displayName = name.substr(0, strlen("ImGui_")) == "ImGui_"
              ? name.substr(   strlen("ImGui_")) : name;

  while(argTypes.size() > 0 && argNames.size() > 0) { // argDefvs may be empty
    size_t typeLen { argTypes.find(',') },
           nameLen { argNames.find(',') },
           defvLen { argDefvs.find('\31') };

    if(argDefvs.substr(0, strlen("ImGui")) == "ImGui") {
      argDefvs.remove_prefix(strlen("ImGui"));
      defvLen -= strlen("ImGui");
    }
    else if(argDefvs.substr(0, strlen("ImDraw")) == "ImDraw") {
      argDefvs.remove_prefix(strlen("Im"));
      defvLen -= strlen("Im");
    }

    args.emplace_back(Argument {
      argTypes.substr(0, typeLen),
      argNames.substr(0, nameLen),
      argDefvs.substr(0, defvLen),
    });

    if(typeLen == std::string_view::npos ||
       nameLen == std::string_view::npos ||
       defvLen == std::string_view::npos)
      break;

    argTypes.remove_prefix(typeLen + 1);
    argNames.remove_prefix(nameLen + 1);
    argDefvs.remove_prefix(defvLen + 1);
  }

  const API::Section *curSection { section };
  do { sections.push_front(curSection); }
  while((curSection = curSection->parent));
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
class ImGui_Image;
class ImGui_ImageSet;
class ImGui_ListClipper;
class ImGui_Resource;
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
    stream << hl(Highlight::Constant) << "nullptr";
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
  stream << hl(Highlight::Type) << type << hl() << ' ' << name;
  if(isEnum())
    return;
  stream << '(';
  CommaSep cs { stream };
  for(const Argument &arg : args) {
    cs << hl(Highlight::Type) << arg.type << hl() << ' ' << arg.name;
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
  auto oldSection { oldSections.begin() };
  auto newSection { func.sections.begin() };
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

struct Reference {
  const Function *func;
  std::string_view range;
};

static std::vector<Reference> parseReferences(const std::string_view &input)
{
  // build a map of known references for fast lookup
  enum CharInfo { InitialChar = 1<<0, ValidChar = 1<<1 };
  static std::unordered_map<std::string_view, const Function *> funcs;
  static char charmap[0x100];
  if(funcs.empty()) {
    for(const Function &func : g_funcs) {
      funcs.emplace(func.displayName, &func);
      funcs.emplace(func.name, &func);
    }
    // build a maps of which characters may be present in a reference
    for(const auto &pair : funcs) {
      charmap[static_cast<unsigned char>(pair.first[0])] |= InitialChar;
      for(const unsigned char c : pair.first)
        charmap[c] |= ValidChar;
    }
  }

  std::vector<Reference> links;

  auto start { input.begin() };
  while(start != input.end()) {
    start = std::find_if(start, input.end(),
      [&](const unsigned char c) { return charmap[c] & InitialChar; });
    if(start == input.end())
      break;
    const auto end { std::find_if_not(start, input.end(),
      [&](const unsigned char c) { return charmap[c] & ValidChar; }) };
    // constructor taking (first, last) iterators is C++20
    const std::string_view word { &*start, static_cast<size_t>(end - start) };
    const auto it { funcs.find(word) };
    if(it != funcs.end())
      links.push_back({ it->second, word });
    start += word.size();
  }

  return links;
}

static void outputHtmlBlock(std::ostream &stream, std::string_view html,
  const bool escape = true)
{
  const auto &links { parseReferences(html) };
  for(auto link { links.begin() }; link != links.end(); ++link) {
    const auto prefixSize { link->range.data() - html.data() };
    const std::string_view prefix { html.substr(0, prefixSize) };
    if(escape)
      outputHtmlText(stream, prefix);
    else
      stream << prefix;
    stream << "<a href=\"#" << link->func->displayName << "\">"
           << link->range << "</a>";
    html.remove_prefix(prefixSize + link->range.size());
  }
  if(escape)
    outputHtmlText(stream, html);
  else
    stream << html;
}

static void outputMarkdown(std::ostream &stream, const std::string_view &text)
{
  if(char *html { cmark_markdown_to_html(text.data(), text.size(), 0) }) {
    outputHtmlBlock(stream, html, false);
    free(html);
  }
  else {
    stream << "<pre>";
    outputHtmlBlock(stream, text);
    stream << "</pre>";
  }
}

static void outputHtmlSlug(std::ostream &stream, const std::string_view &text)
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
    background-color: #0D0D0D;
    color: #d9d3d3;
    font-size: 15px;
    line-height: 20px;
    margin: 0;
    overflow-anchor: none;
  }
  body, pre, code {
    font-family: ui-monospace, SFMono-Regular, SF Mono, Menlo, Consolas,
                 Liberation Mono, monospace;
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
  aside p, aside li a, main { padding-left: 1em; }
  h2, h3, h4, h5, h6, pre { margin: 1em 0 1rem 0; }
  h1 { font-size: 2.3em; }
  h2 { font-size: 1.8em; }
  h3 { font-size: 1.4em; }
  h4 { font-size: 1.15em; }
  h5 { font-size: 1.0em; }
  h6 { font-size: 0.9em; }
  h2:before { content: '〉'; color: #6f6f6f; }
  ol { padding-left: 2em; }
  ul { padding-left: 1em; }
  li ul { list-style-type: square; }
  a { text-decoration: none; }
  a, summary { color: #00ff87; /* SpringGreen1 */ }
  a:hover, summary:hover { text-decoration: underline; }
  details { margin-left: 20px; }
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
  table { white-space: pre-wrap; }
  code, code a { color: white; }
  table code:hover { text-decoration: underline; cursor: copy; }
  table code:active, aside a:hover { background-color: #3a3a3a; }
  tr + tr td { border-top: 1px solid #555; }
  pre { overflow: auto; }
  pre code {
    background-color: black;
    border-radius: 5px;
    border: 1px solid #6f6f6f;
    display: inline-block;
    padding: 0.5em;
  }
  .st { color: #87afff; /* SkyBlue2 */ }
  .ss, .string   { color: #5faf5f; /* DarkSeaGreen4 */ }
  .sn, .number   { color: #5f87d7; /* SteelBlue3    */ }
  .sr, .keyword  { color: #d7875f; /* LightSalmon3  */ }
       .comment  { color: #b2b2b2; /* Grey70        */ }
       .built_in { color: #87d75f; /* DarkOliveGreen3 */ }
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
    outputHtmlSlug(stream, section->title); stream << "\">";
    outputHtmlText(stream, section->title);
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
        outputHtmlSlug(stream, (*slugIt)->title);
      }
      stream << "\">";
      outputHtmlText(stream, section->title);
      stream << "</h" << level << '>';

      if(section->help)
        outputMarkdown(stream, section->help);
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
      outputMarkdown(stream, func.doc);

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
  for(const API *func { API::head() }; func; func = func->m_next)
    g_funcs.push_back(func);
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
