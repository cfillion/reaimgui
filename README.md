# ReaImGui: ReaScript binding for dear imgui

[![Build status](https://ci.appveyor.com/api/projects/status/9umkecgrs4sa8odf/branch/master?svg=true)](https://ci.appveyor.com/project/cfillion/reaimgui/branch/master)

This REAPER extension is distributed through [ReaPack](https://reapack.com)
through the default [ReaTeam Extensions](https://github.com/ReaTeam/Extensions)
repository.

## Build from source

Clone the repository and submodules:

    git clone --recursive https://github.com/cfillion/reaimgui.git

### Prerequisites

Software requirements:

- [CMake](https://cmake.org/) 3.19 or newer
- C++17 compiler (MSVC on Windows)

#### Linux

Install the following libraries (and development headers if your system provides
them separately):

- [Boost](https://www.boost.org/)
- [GDK3](https://developer.gnome.org/gdk3/stable/) (3.16 or newer)
- [libepoxy](https://github.com/anholt/libepoxy)

#### macOS

Install Boost using [Homebrew](https://brew.sh) (recommended).
The build tools can be installed using `xcode-select --install` or the Xcode IDE.

#### Windows

MSVC can be installed with the [Build Tools for Visual Studio](
https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools)
or the Visual Studio IDE.

Use the x64 or x86 Native Tools Command Prompt for VS 20XX matching the target
architecture when configuring or building ReaImGui.

Install [vcpkg](https://docs.microsoft.com/cpp/build/vcpkg) in any directory:

    git clone https://github.com/Microsoft/vcpkg.git C:\path\to\vcpkg
    C:\path\to\vcpkg\bootstrap-vcpkg.bat

Set the `VCPKG_ROOT` and `VCPKG_DEFAULT_TRIPLET` environment variables
(only required when running `vcpkg install` or creating a new build tree):

    set VCPKG_ROOT=C:\path\to\vcpkg
    set VCPKG_DEFAULT_TRIPLET=%PLATFORM%-windows-static

Install ReaImGui's build dependencies:

    set /p reaimgui-deps=<vendor\vcpkg-deps.txt
    %VCPKG_ROOT%\vcpkg install %reaimgui-deps%

### Build configuration

Create and configure a new build tree inside of the `build` directory.

    cmake -B build -DCMAKE_BUILD_TYPE=Debug

Using the [Ninja](https://ninja-build.org/) generator is recommended for
best performances:

    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

Alternatively, multiple build trees can be created if desired:

    cmake -B build/debug    -DCMAKE_BUILD_TYPE=Debug
    cmake -B build/release  -DCMAKE_BUILD_TYPE=Release
    cmake -B build/portable -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_INSTALL_PREFIX=/path/to/reaper/portable/install

The vcpkg install is automatically detected and configured from the `VCPKG_ROOT`
and `VCPKG_DEFAULT_TRIPLET` environment variables when creating a build tree on
Windows.

### Compile and install

To compile a build tree:

    cmake --build build

To install ReaImGui into your REAPER installation after building:

    cmake --build build --target install

The following targets are available:

- **`all`**: Build ReaImGui (default target)
- **`clean`**: Delete all generated files
  (can be run before building another target using `--clean-first`)
- **`install`**: Build and install ReaImGui into REAPER's resource directory
  (as specified in `CMAKE_INSTALL_PREFIX`)
- **`test`**: Build and run the test suite

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

macOS 10.14 or older, Xcode 9 is required for producing 32-bit builds.

    cmake -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_ARCHITECTURES=i386 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9
