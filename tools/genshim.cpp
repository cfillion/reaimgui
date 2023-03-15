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

#include <algorithm>
#include <boost/preprocessor/stringize.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#ifdef HAVE_STD_FILESYSTEM
#  include <filesystem>
#  include <fstream>
  namespace fs {
    using namespace std::filesystem;
    using error_code = std::error_code;
    using ifstream = std::ifstream;
  }
#else
#  include <boost/filesystem.hpp>
#  include <boost/filesystem/fstream.hpp>
  namespace fs {
    using namespace boost::filesystem;
    using error_code = boost::system::error_code;
  }
#endif

using Version = uint32_t;

constexpr const char *GENERATED_FOR { "Generated for ReaImGui v" REAIMGUI_VERSION };

struct Shim {
  Version ver;
  fs::path path;
  bool operator<(const Shim &o) const { return ver > o.ver; }
};

static bool packVersion(const std::string &ver, Version *packed)
{
  constexpr int segs { sizeof(Version) };
  size_t pos {};
  *packed = 0;
  for(int i { 1 }; i <= segs; ++i) {
    size_t sepPos { ver.find('.', pos) };
    if(sepPos == std::string::npos)
      sepPos = ver.size();
    int seg;
    try {
      seg = std::stoi(ver.substr(pos, sepPos - pos));
    }
    catch(const std::invalid_argument &) {
      std::cerr << ver << " has an non-numerical segment" << std::endl;
      return false;
    }
    if(seg & ~0xff) {
      std::cerr << ver << " has version segment " << seg
                << "doesn't fit in a byte" << std::endl;
      return false;
    }
    *packed |= seg << 8 * (segs - i);
    if(sepPos == ver.size())
      return true;
    else
      pos = sepPos + 1;
  }
  std::cerr << ver << " has more than " << segs << " segments" << std::endl;
  return false;
}

static void hexVersion(std::ostream &stream, const Version ver)
{
  stream << "0x"
         << std::setfill('0') << std::setw(sizeof(Version) * 2) << std::hex
         << ver;
}

static bool loadShims(const fs::path &dir, std::vector<Shim> &shims)
{
  fs::error_code ec;
  for(const auto &entry : fs::directory_iterator { dir, ec }) {
    const auto &fn { entry.path() };
    if(fs::is_directory(fn))
      continue;
    Version version;
    if(!packVersion(fn.stem().string(), &version)) {
      std::cerr << fn.string() << ": failed to load shim file" << std::endl;
      return false;
    }
    shims.push_back({ version, fn });
  }
  if(ec) {
    std::cerr << dir.string() << ": " << ec.message() << std::endl;
    return false;
  }
  std::sort(shims.begin(), shims.end());
  return true;
}

static int luaShims(std::ostream &stream, const std::vector<Shim> &shims)
{
  Version version;
  packVersion(BOOST_PP_STRINGIZE(REAIMGUI_VERSION_MAJOR) "."
              BOOST_PP_STRINGIZE(REAIMGUI_VERSION_MINOR) "."
              BOOST_PP_STRINGIZE(REAIMGUI_VERSION_PATCH) "."
              BOOST_PP_STRINGIZE(REAIMGUI_VERSION_TWEAK), &version);

  stream << "-- " << GENERATED_FOR << "\n\n"
         << "local shims = {\n";

  for(const Shim &shim : shims) {
    stream << "  { version=";
    hexVersion(stream, shim.ver);
    stream << ", apply=function() -- v" << shim.path.stem().string() << '\n';
    fs::ifstream file { shim.path };
    if(!file) {
      std::cerr << "failed to open shim file " << shim.path.string() << std::endl;
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
  if(argc < 3) {
    std::cerr << "Usage: " << argv[0] << " LANG SHIMS_DIR" << std::endl;
    return 1;
  }

  const std::string_view lang { argv[1] }, dir { argv[2] };
  std::vector<Shim> shims;
  if(!loadShims(dir.data(), shims))
    return 1;
  else if(shims.empty()) {
    std::cerr << "no shims found in '" << dir << "'" << std::endl;
    return 1;
  }

  if(lang == "lua")
    return luaShims(std::cout, shims);
  else {
    std::cerr << "don't know how to generate shims for '"
              << lang << "'" << std::endl;
    return 1;
  }
}
