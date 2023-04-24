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

#include <config.hpp>
#include <version.hpp>

#include "../src/win32_unicode.hpp"

#include <algorithm>
#include <boost/preprocessor/stringize.hpp>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

constexpr const char *GENERATED_FOR
  { "Generated for ReaImGui v" REAIMGUI_VERSION };

class Version {
public:
  Version(const std::string_view &);
  bool operator>(const Version &o) const { return m_value > o.m_value; }

  auto value() const { return m_value; }
  std::string_view string() const { return m_string; }

private:
  std::string_view m_string;
  uint32_t m_value;
};

struct Shim {
  Version version;
  std::string_view path;
  bool operator<(const Shim &o) const { return version > o.version; }
};

Version::Version(const std::string_view &ver)
  : m_string { ver }, m_value {}
{
  constexpr int segs { sizeof(m_value) };
  size_t pos {};
  for(int i { 1 }; i <= segs; ++i) {
    size_t sepPos { ver.find('.', pos) };
    if(sepPos == std::string::npos)
      sepPos = ver.size();
    unsigned char seg;
    const std::string_view segStr { ver.substr(pos, sepPos - pos) };
    const auto result
      { std::from_chars(segStr.data(), segStr.data() + segStr.size(), seg) };
    switch(result.ec) {
    case std::errc::invalid_argument:
      throw std::runtime_error
        { "segment " + std::string { segStr } + " is not a number" };
    case std::errc::result_out_of_range:
      throw std::runtime_error
        { "segment " + std::string { segStr } + " does not fit in a byte" };
    default:
      break;
    }
    m_value |= seg << 8 * (segs - i);
    if(sepPos == ver.size())
      return;
    else
      pos = sepPos + 1;
  }
  throw std::runtime_error
    { "version has more than " + std::to_string(segs) + " segments" };
}

static std::string_view basename(const std::string_view &fn)
{
  size_t start { fn.rfind('/') }, end { fn.rfind('.') };
  start = start == std::string_view::npos ? 0 : start + 1;
  return fn.substr(start, end - start);
}

static void hexVersion(std::ostream &stream, const Version ver)
{
  stream << "0x"
         << std::setfill('0') << std::setw(sizeof(decltype(ver.value())) * 2)
         << std::hex << ver.value();
}

// FIXME: std::expected and std::span
static std::vector<Shim> loadShims(const char *files[], const int size)
{
  std::vector<Shim> shims;

  for(int i {}; i < size; ++i) {
    const std::string_view fn { files[i] };
    try {
      shims.push_back({ basename(fn), fn });
    }
    catch(const std::runtime_error &e) {
      throw std::runtime_error { std::string { fn } + ": " + e.what() };
    }
  }

  std::sort(shims.begin(), shims.end());

  return shims;
}

static int luaShims(std::ostream &stream, const std::vector<Shim> &shims)
{
  Version version {
    BOOST_PP_STRINGIZE(REAIMGUI_VERSION_MAJOR) "."
    BOOST_PP_STRINGIZE(REAIMGUI_VERSION_MINOR) "."
    BOOST_PP_STRINGIZE(REAIMGUI_VERSION_PATCH) "."
    BOOST_PP_STRINGIZE(REAIMGUI_VERSION_TWEAK)
  };

  stream << "-- " << GENERATED_FOR << "\n\n"
         << "local shims = {\n";

  for(const Shim &shim : shims) {
    stream << "  { version=";
    hexVersion(stream, shim.version);
    stream << ", apply=function() -- v" << shim.version.string() << '\n';
    std::ifstream file { WIDEN(shim.path.data()) };
    if(!file) {
      std::cerr << "failed to open shim file " << shim.version.string() << std::endl;
      return 1;
    }
    std::string line;
    while(std::getline(file, line))
      stream << "    " << line << '\n';
    stream << "  end },\n";
  }

  stream << R"(}

local function parseVersion(ver)
  local segs, pos, packed = 4, 1, 0
  ver = tostring(ver)
  for i = 1, segs do
    local sep_pos = ver:find('.', pos, true)
    local seg = tonumber(ver:sub(pos, sep_pos and sep_pos - 1 or nil))
    if not seg or (seg & ~0xff) ~= 0 then break end -- invalid
    packed = packed | (seg << 8 * (segs - i))
    if not sep_pos then return packed end
    pos = sep_pos + 1
  end
  error(('invalid version string "%s"'):format(ver))
end

return function(compat_version)
  local version = parseVersion(compat_version)
  assert(version <= )"; hexVersion(stream, version); stream << R"(,
    ('reaimgui )" REAIMGUI_VERSION
    R"( is too old (script requires %s)'):format(compat_version))
  for _, shim in ipairs(shims) do
    if shim.version <= version then break end
    shim.apply()
  end
end
)";

  return 0;
}

int main(int argc, const char *argv[])
{
  using namespace std::literals::string_literals;

  if(argc < 3) {
    std::cerr << "Usage: " << argv[0] << " LANG SHIM_FILE...\n";
    return 1;
  }

  try {
    std::vector<Shim> shims { loadShims(argv + 2, argc - 2) };

    const std::string_view lang { argv[1] };
    if(lang == "lua")
      return luaShims(std::cout, shims);
    else {
      throw std::runtime_error
        { "don't know how to generate shims for '"s + lang.data() + "'" };
    }
  }
  catch(std::runtime_error &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
