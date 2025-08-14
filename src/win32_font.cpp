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
#include <dwrite_3.h>
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

  if(font->IsSymbolFont())
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

static std::string getFilename(CComPtr<IDWriteFontFile> file)
{
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

static bool operator==(const DWRITE_FONT_AXIS_VALUE &a, const DWRITE_FONT_AXIS_VALUE &b)
{
  return !memcmp(&a, &b, sizeof(DWRITE_FONT_AXIS_VALUE));
}

static unsigned short findVariation(CComPtr<IDWriteFactory> factory,
  CComPtr<IDWriteFontFace> face, CComPtr<IDWriteFontFile> file)
{
  // Windows 10 Build 16299
  CComQIPtr<IDWriteFactory5> factory5 {factory};
  CComQIPtr<IDWriteFontFace5> face5 {face};
  if(!factory5 || !face5 || !face5->HasVariations())
    return 0;

  const auto targetIndex {face->GetIndex()};
  std::vector<DWRITE_FONT_AXIS_VALUE> targetValues {face5->GetFontAxisValueCount()};
  if(FAILED(face5->GetFontAxisValues(targetValues.data(), targetValues.size())))
    return 0;

  // DirectWrite lacks an API to get the index of a variation's named instance
  // so we iterate through all instances contained in the font file to find it

  CComPtr<IDWriteFontSetBuilder1> builder;
  if(FAILED(factory5->CreateFontSetBuilder(&builder)))
    throw reascript_error {"failed to create font variation set builder"};
  if(FAILED(builder->AddFontFile(file)))
    throw reascript_error {"failed to read variations from font file"};
  CComPtr<IDWriteFontSet> set;
  if(FAILED(builder->CreateFontSet(&set)))
    throw reascript_error {"failed to create font variation set"};
  CComQIPtr<IDWriteFontSet1> set1 {set}; // same min OS version as face5

  const auto count {std::min(set1->GetFontCount(), 0xFFFEU)}; // FT's limit
  std::vector<DWRITE_FONT_AXIS_VALUE> refValues {targetValues.size()};
  for(unsigned short i {}, varIndex {}; i < count; ++i) {
    CComPtr<IDWriteFontFaceReference1> ref;
    if(FAILED(set1->GetFontFaceReference(i, &ref)))
      throw reascript_error {"failed to get font variation reference"};

    if(ref->GetFontFaceIndex() != targetIndex)
      continue;

    ++varIndex; // index is 1-based, increment first

    if(ref->GetFontAxisValueCount() != targetValues.size() ||
        FAILED(ref->GetFontAxisValues(refValues.data(), refValues.size())))
      continue;

    if(refValues == targetValues)
      return varIndex;
  }

  return 0;
}

struct DWObjects {
  FuncImport<decltype(DWriteCreateFactory)> _DWriteCreateFactory;
  CComPtr<IDWriteFactory> factory;
  CComPtr<IDWriteFontCollection> collection;
  CComPtr<IDWriteFontFallback> fallback;

  DWObjects();
  operator bool() const { return _DWriteCreateFactory; }
};

DWObjects::DWObjects()
  : _DWriteCreateFactory {L"Dwrite", "DWriteCreateFactory"}
{
  if(!_DWriteCreateFactory)
    return;

  if(FAILED(_DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&factory))))
    throw reascript_error {"failed to initialize DirectWrite"};

   if(FAILED(factory->GetSystemFontCollection(&collection)))
     throw reascript_error {"failed to get the system font collection"};

  if(CComQIPtr<IDWriteFactory2> factory2 {factory}) {
    if(FAILED(factory2->GetSystemFontFallback(&fallback)))
      throw reascript_error {"failed to get the system font fallback"};
  }
}

void SysFont::initPlatform()
{
  static std::weak_ptr<void> g_shared;

  if(g_shared.expired())
    g_shared = m_platform = std::make_shared<DWObjects>();
  else
    m_platform = g_shared.lock();
}

std::optional<FontSource> SysFont::resolve(const unsigned int codepoint) const
{
  const auto &dwrite {*std::static_pointer_cast<DWObjects>(m_platform)};
  const std::wstring &family {translateGenericFont(m_family)};

  // Windows Vista without Platform Update
  if(!dwrite) {
    if(codepoint)
      return std::nullopt;
    return resolveGDI(family.c_str(), m_styles);
    //throw reascript_error {"DirectWrite is not installed on this system"};
  }

  CComPtr<IDWriteFont> match;

  // Windows 8.1: use the system fallback mechanism
  if(dwrite.fallback) {
    CodepointSource source {codepoint};
    unsigned int mappedLen; float scale;
    dwrite.fallback->MapCharacters(&source, 0, 1, dwrite.collection,
      family.c_str(), getWeight(m_styles), getStyle(m_styles),
      DWRITE_FONT_STRETCH_NORMAL, &mappedLen, &match, &scale);
    if(match && match->IsSymbolFont())
      match.Release();
  }

  // find a font matching the requested family name
  // (skip if we already know it doesn't contain the requested codepoint)
  if(!match && !codepoint) {
    BOOL found {}; unsigned int i;
    dwrite.collection->FindFamilyName(family.c_str(), &i, &found);

    CComPtr<IDWriteFontFamily> family;
    if(found && SUCCEEDED(dwrite.collection->GetFontFamily(i, &family)))
      match = findMatch(family, m_styles, codepoint);
  }

  // find any font containing the requested glyph
  if(!match) {
    const unsigned int families {dwrite.collection->GetFontFamilyCount()};
    for(unsigned int i {}; i < families; ++i) {
      CComPtr<IDWriteFontFamily> family;
      if(SUCCEEDED(dwrite.collection->GetFontFamily(i, &family)))
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

  UINT32 filesCount {1}; // we don't support more than one file per font source
  CComPtr<IDWriteFontFile> file;
  face->GetFiles(&filesCount, &file);

  FontSource src;
  src.m_data = getFilename(file);
  src.m_index = face->GetIndex() & 0xFFFF;
  src.m_index |= findVariation(dwrite.factory, face, file) << 16;
  src.m_styles = m_styles;
  if(match->GetWeight() > DWRITE_FONT_WEIGHT_REGULAR
      && !(match->GetSimulations() & DWRITE_FONT_SIMULATIONS_BOLD))
    src.m_styles &= ~ReaImGuiFontFlags_Bold;
  if(match->GetStyle() == DWRITE_FONT_STYLE_ITALIC)
    src.m_styles &= ~ReaImGuiFontFlags_Italic;

  return src;
}
