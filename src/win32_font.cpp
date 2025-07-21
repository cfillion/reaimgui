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

#include "font.hpp"

#include "error.hpp"
#include "import.hpp"
#include "win32_unicode.hpp"

#include <atlbase.h>
#include <dwrite_2.h>
#include <imgui_internal.h>
#include <strsafe.h>

class SelectFont {
public:
  SelectFont(LOGFONT *);
  ~SelectFont();

  operator bool() const { return !!m_font; }
  HDC dc() const { return m_dc; }

private:
  HDC m_dc;
  HFONT m_font;
};

SelectFont::SelectFont(LOGFONT *desc)
  : m_dc {CreateCompatibleDC(nullptr)}, m_font {CreateFontIndirect(desc)}
{
  SelectObject(m_dc, m_font);
}

SelectFont::~SelectFont()
{
  DeleteDC(m_dc);
  DeleteObject(m_font);
}

static int CALLBACK enumStyles(const LOGFONT *desc,
  const TEXTMETRIC *, DWORD, LPARAM lParam)
{
  auto src {reinterpret_cast<FontSource *>(lParam)};
  if(desc->lfWeight > FW_NORMAL) src->m_styles &= ~ReaImGuiFontFlags_Bold;
  if(desc->lfItalic) src->m_styles &= ~ReaImGuiFontFlags_Italic;
  return 1;
}

static std::optional<FontSource> resolveGDI(const wchar_t *family, int styles)
{
  LOGFONT desc {
    .lfWeight       = styles & ReaImGuiFontFlags_Bold ? FW_BOLD : FW_NORMAL,
    .lfItalic       = !!(styles & ReaImGuiFontFlags_Italic),
    .lfOutPrecision = OUT_TT_ONLY_PRECIS,
  };
  StringCchCopy(desc.lfFaceName, LF_FACESIZE, family);

  SelectFont sel {&desc};
  if(!sel)
    return std::nullopt;

  const DWORD dataSize {GetFontData(sel.dc(), 0, 0, nullptr, 0)};
  if(dataSize == GDI_ERROR)
    return std::nullopt;
  std::vector<unsigned char> fontData(dataSize);
  GetFontData(sel.dc(), 0, 0, fontData.data(), fontData.size());

  FontSource src {std::move(fontData), 0, styles};

  GetTextFace(sel.dc(), LF_FACESIZE, desc.lfFaceName);
  EnumFontFamiliesEx(sel.dc(), &desc, &enumStyles,
    reinterpret_cast<LPARAM>(&src), 0);

  return src;
}

class CodepointSource : public IDWriteTextAnalysisSource {
public:
  CodepointSource(unsigned int);

  ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
  ULONG STDMETHODCALLTYPE Release() override { return 0; }
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **object) override;

  HRESULT STDMETHODCALLTYPE GetTextAtPosition(UINT32, const wchar_t **, UINT32 *) override;
  HRESULT STDMETHODCALLTYPE GetTextBeforePosition(UINT32, const wchar_t **, UINT32 *) override;
  DWRITE_READING_DIRECTION STDMETHODCALLTYPE GetParagraphReadingDirection() override;
  HRESULT STDMETHODCALLTYPE GetLocaleName(UINT32, UINT32 *, const wchar_t **) override;
  HRESULT STDMETHODCALLTYPE GetNumberSubstitution(UINT32, UINT32 *, IDWriteNumberSubstitution **) override;

private:
  wchar_t m_text[2 + 1];
  wchar_t m_locale[LOCALE_NAME_MAX_LENGTH];
  unsigned int m_length;
};

CodepointSource::CodepointSource(unsigned int codepoint)
{
  if(!codepoint)
    codepoint = L'A';

  char utf8[5];
  ImTextCharToUtf8(utf8, codepoint);
  m_length = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, m_text, std::size(m_text)) - 1;

  GetUserDefaultLocaleName(m_locale, LOCALE_NAME_MAX_LENGTH);
}

HRESULT CodepointSource::QueryInterface(REFIID riid, void **object)
{
  if(riid == IID_IUnknown || riid == __uuidof(IDWriteTextAnalysisSource)) {
    *object = this;
    AddRef();
    return S_OK;
  }

  *object = nullptr;
  return E_NOINTERFACE;
}

HRESULT CodepointSource::GetTextAtPosition(UINT32 i, const wchar_t **text, UINT32 *length)
{
  if(i >= m_length)
    return E_INVALIDARG;

  *text = m_text + i;
  *length = m_length - i;
  return S_OK;
}

HRESULT CodepointSource::GetTextBeforePosition(UINT32 i, const wchar_t **text, UINT32 *length)
{
  if(i < 1)
    return E_INVALIDARG;

  *text = m_text;
  *length = i;
  return S_OK;
}

DWRITE_READING_DIRECTION CodepointSource::GetParagraphReadingDirection()
{
  return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
}

HRESULT CodepointSource::GetLocaleName(UINT32 i, UINT32 *length, const wchar_t **locale)
{
  if(i >= m_length)
    return E_INVALIDARG;

  *length = m_length - i;
  *locale = m_locale;
  return S_OK;
}

HRESULT CodepointSource::GetNumberSubstitution(UINT32, UINT32 *, IDWriteNumberSubstitution **)
{
  return E_NOTIMPL;
}

static DWRITE_FONT_WEIGHT getWeight(const int styles)
{
  return styles & ReaImGuiFontFlags_Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR;
}

static DWRITE_FONT_STYLE getStyle(const int styles)
{
  return styles & ReaImGuiFontFlags_Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
}

static std::wstring translateGenericFont(const std::string &family)
{
  constexpr std::pair<const char *, const wchar_t *> genericMap[] {
    {SysFont::SANS_SERIF, L"Segoe UI"        },
    {SysFont::SERIF,      L"Times New Roman" },
    {SysFont::MONOSPACE,  L"Consolas"        },
    {SysFont::CURSIVE,    L"Segoe Script"    },
    {SysFont::FANTASY,    L"Comic Sans MS"   },
  };

  for(const auto &generic : genericMap) {
    if(!_stricmp(generic.first, family.c_str()))
      return generic.second;
  }

  return widen(family);
}

static CComPtr<IDWriteFont> findMatch(CComPtr<IDWriteFontFamily> family,
  int styles, const unsigned int codepoint)
{
  CComPtr<IDWriteFont> font;
  if(FAILED(family->GetFirstMatchingFont(
      getWeight(styles), DWRITE_FONT_STRETCH_UNDEFINED, getStyle(styles), &font)))
    return nullptr;

  if(codepoint) {
    BOOL found;
    CComQIPtr<IDWriteFont2> font2 {font};
    if(font2)
      font2->HasCharacter(codepoint, &found);
    else
      font->HasCharacter(codepoint, &found);
    if(!found)
      return nullptr;
  }

  return font;
}

static std::string getFilename(CComPtr<IDWriteFontFace> face)
{
  UINT32 filesCount {1}; // we don't support more than one file per font source
  CComPtr<IDWriteFontFile> file;
  face->GetFiles(&filesCount, &file);

  const void *refKey; UINT32 refKeySize;
  file->GetReferenceKey(&refKey, &refKeySize);

  CComPtr<IDWriteFontFileLoader> loader;
  file->GetLoader(&loader);
  CComQIPtr<IDWriteLocalFontFileLoader> localLoader {loader};
  if(!localLoader)
    throw reascript_error {"failed to query local font file loader"};

  UINT32 pathLength;
  if(FAILED(localLoader->GetFilePathLengthFromKey(refKey, refKeySize, &pathLength)))
    throw reascript_error {"could not query local font file path"};
  std::wstring path(pathLength, L'\0');
  localLoader->GetFilePathFromKey(refKey, refKeySize, path.data(), path.size() + 1);

  return narrow(path);
}

std::optional<FontSource> SysFont::resolve(const unsigned int codepoint) const
{
  static FuncImport<decltype(DWriteCreateFactory)>
    _DWriteCreateFactory {L"Dwrite", "DWriteCreateFactory"};

  const std::wstring &family {translateGenericFont(m_family)};

  // Windows Vista without Platform Update
  if(!_DWriteCreateFactory) {
    if(codepoint)
      return std::nullopt;
    return resolveGDI(family.c_str(), m_styles);
    //throw reascript_error {"DirectWrite is not installed on this system"};
  }

  CComPtr<IDWriteFactory> factory;
  if(FAILED(_DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&factory))))
    throw reascript_error {"failed to initialize DirectWrite"};

  CComPtr<IDWriteFontCollection> collection;
   if(FAILED(factory->GetSystemFontCollection(&collection)))
     throw reascript_error {"failed to get the system font collection"};

  CComPtr<IDWriteFont> match;

  // Windows 8.1: use the system fallback mechanism
  if(CComQIPtr<IDWriteFactory2> factory2 {factory}) {
    CComPtr<IDWriteFontFallback> fallback;
    if(FAILED(factory2->GetSystemFontFallback(&fallback)))
      throw reascript_error {"failed to get the system font fallback"};

    CodepointSource source {codepoint};
    unsigned int mappedLen; float scale;
    fallback->MapCharacters(&source, 0, 1, collection, family.c_str(),
      getWeight(m_styles), getStyle(m_styles), DWRITE_FONT_STRETCH_NORMAL,
      &mappedLen, &match, &scale);
  }

  // find a font matching the requested family name
  // (skip if we already know it doesn't contain the requested codepoint)
  if(!match && !codepoint) {
    BOOL found {}; unsigned int i;
    collection->FindFamilyName(family.c_str(), &i, &found);

    CComPtr<IDWriteFontFamily> family;
    if(found && SUCCEEDED(collection->GetFontFamily(i, &family)))
      match = findMatch(family, m_styles, codepoint);
  }

  // find any font containing the requested glyph
  if(!match) {
    const unsigned int families {collection->GetFontFamilyCount()};
    for(unsigned int i {}; i < families; ++i) {
      CComPtr<IDWriteFontFamily> family;
      if(SUCCEEDED(collection->GetFontFamily(i, &family)))
        match = findMatch(family, m_styles, codepoint);
      if(match)
        break;
    }
  }

  // the user has no fonts?
  if(!match)
    return std::nullopt;

  CComPtr<IDWriteFontFace> face;
  if(FAILED(match->CreateFontFace(&face)))
    throw reascript_error {"failed to create the system font face"};

  FontSource src;
  src.m_data = getFilename(face);
  src.m_index = face->GetIndex();
  src.m_styles = m_styles;
  if(match->GetWeight() > DWRITE_FONT_WEIGHT_REGULAR
      && !(match->GetSimulations() & DWRITE_FONT_SIMULATIONS_BOLD))
    src.m_styles &= ~ReaImGuiFontFlags_Bold;
  if(match->GetStyle() == DWRITE_FONT_STYLE_ITALIC)
    src.m_styles &= ~ReaImGuiFontFlags_Italic;

  return src;
}
