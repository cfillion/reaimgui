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

#include "../src/win32_unicode.hpp"

#include <atlbase.h>
#include <d3dcompiler.h>
#include <iomanip>
#include <iostream>

int main(int argc, const char *argv[])
{
  if(argc != 3) {
    std::cerr << "Usage: gend3dshader profile file.hlsl" << std::endl;
    return 1;
  }

  const char *target {argv[1]}, *filename {argv[2]};

  CComPtr<ID3DBlob> blob, errors;
  constexpr unsigned int flags {
    D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS
  };
  D3DCompileFromFile(widen(filename).c_str(), nullptr, nullptr,
    "main", target, flags, 0, &blob, &errors);

  if(errors) {
    std::cerr << static_cast<const char *>(errors->GetBufferPointer());
    return 1;
  }

  if(!blob) {
    std::cerr << "D3DCompile did not produce an output" << std::endl;
    return 1;
  }

  const uint8_t *data {static_cast<uint8_t *>(blob->GetBufferPointer())};
  const size_t size {blob->GetBufferSize()};

  for(size_t i {0}; i < size; ++i) {
    if(i > 0) {
      if(!(i % 8))
        std::cout << std::endl;
      else
        std::cout << ' ';
    }
    std::cout << "0x"
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<short>(data[i]) << ",";
  }
  std::cout << std::endl;

  return 0;
}
