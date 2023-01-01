# ReaImGui: ReaScript binding for Dear ImGui

[![Build status](https://ci.appveyor.com/api/projects/status/9umkecgrs4sa8odf/branch/master?svg=true)](https://ci.appveyor.com/project/cfillion/reaimgui/branch/master)

This REAPER extension is distributed via [ReaPack](https://reapack.com)
through the default [ReaTeam Extensions](https://github.com/ReaTeam/Extensions)
repository.

https://forum.cockos.com/showthread.php?t=250419

## Build from source

Clone the repository and submodules:

    git clone --recursive --shallow-submodules https://github.com/cfillion/reaimgui.git

### Prerequisites

Software requirements:

- [CMake](https://cmake.org/) 3.19 or newer
- C++17 compiler (MSVC on Windows)
- PHP (Linux and macOS only)

#### Linux

Install the following libraries (and development headers if your system provides
them separately):

- [Boost](https://www.boost.org/) (1.71 or newer)
- [Fontconfig](https://www.fontconfig.org)
- [FreeType](https://www.freetype.org/)
- [GDK3](https://developer.gnome.org/gdk3/stable/) (3.22 or newer)
- [GoogleTest](https://github.com/google/googletest)
- [libepoxy](https://github.com/anholt/libepoxy)
- [libjpeg-turbo](https://www.libjpeg-turbo.org/)
- [libpng](http://www.libpng.org/pub/png/libpng.html)

#### macOS

Using [Homebrew](https://brew.sh) (recommended):
```sh
brew install boost cmark freetype googletest jpeg-turbo libpng
```
The build tools can be installed using `xcode-select --install` or the Xcode IDE.

#### Windows

MSVC can be installed with the [Build Tools for Visual Studio](
https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools)
or the Visual Studio IDE.

Use the x64 or x86 Native Tools Command Prompt for VS 20XX matching the target
architecture when configuring or building ReaImGui.

Install [vcpkg](https://docs.microsoft.com/cpp/build/vcpkg) in any directory:

    git clone https://github.com/Microsoft/vcpkg.git C:\path\to\vcpkg

Set `VCPKG_TARGET_TRIPLET` and `CMAKE_TOOLCHAIN_FILE` when creating the build
tree:

    -DVCPKG_TARGET_TRIPLET=%PLATFORM%-windows-static
    -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake

### Build configuration

Create and configure a new build tree inside of the `build` directory.

    cmake -B build -DCMAKE_BUILD_TYPE=Debug

Using the [Ninja](https://ninja-build.org/) generator is recommended for
best performance:

    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

Alternatively, multiple build trees can be created if desired:

    cmake -B build/debug    -DCMAKE_BUILD_TYPE=Debug
    cmake -B build/release  -DCMAKE_BUILD_TYPE=Release
    cmake -B build/portable -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_INSTALL_PREFIX=/path/to/reaper/portable/install

### Compile and install

To compile a build tree:

    cmake --build build

To install ReaImGui into your REAPER installation after building:

    cmake --build build --target install

The following targets are available:

- **`all`**: Build ReaImGui and generate bindings (default target)
- **`bindings`**: Generate all bindings
  - **`cpp_binding`**
  - **`python_binding`**
- **`clean`**: Delete all generated files
  (can be run before building another target using `--clean-first`)
- **`install`**: Build and install ReaImGui into REAPER's resource directory
  (as specified in `CMAKE_INSTALL_PREFIX`)
- **`reaimgui`**: Build ReaImGui (without bindings)

### Cross-compilation

#### Linux

`g++-$TOOLCHAIN_PREFIX` will be used when compiling for architectures other than
i686. Libraries for the target architecture are expected to be in
`/usr/{include,lib}/$TOOLCHAIN_PREFIX`, `/usr/$TOOLCHAIN_PREFIX/{include,lib}`
or `/usr/lib32`.

    ARCH=i686 TOOLCHAIN_PREFIX=i386-linux-gnu \
      cmake -B build/i686 -DCMAKE_TOOLCHAIN_FILE=cmake/linux-cross.cmake

    ARCH=armv7l TOOLCHAIN_PREFIX=arm-linux-gnueabihf \
      cmake -B build/arm32 -DCMAKE_TOOLCHAIN_FILE=cmake/linux-cross.cmake

    ARCH=aarch64 TOOLCHAIN_PREFIX=aarch64-linux-gnu \
      cmake -B build/arm64 -DCMAKE_TOOLCHAIN_FILE=cmake/linux-cross.cmake

#### macOS

macOS 10.14 (or older) and Xcode 9 are required for producing 32-bit builds.

    cmake -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_ARCHITECTURES=i386 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9
