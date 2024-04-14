# ReaImGui: ReaScript binding for Dear ImGui

[![Build status](https://ci.appveyor.com/api/projects/status/9umkecgrs4sa8odf/branch/master?svg=true)](https://ci.appveyor.com/project/cfillion/reaimgui/branch/master)

This REAPER extension is distributed via [ReaPack](https://reapack.com)
through the default [ReaTeam Extensions](https://github.com/ReaTeam/Extensions)
repository.

https://forum.cockos.com/showthread.php?t=250419

## Build from source

### Prerequisites

Software requirements:

- [Meson](https://mesonbuild.com/) (1.1 or newer)
- C++20 compiler* (C++17 is supported for Linux and macOS builds)
- PHP (optional, for preprocessing of gfx2imgui.lua)

(* The C++ compiler must be ABI-compatible with MSVC on Windows)

Library requirements:

By default these will be picked up from the system or downloaded if missing.

- [Boost](https://www.boost.org/) (1.81 or newer)
- [fmt](https://fmt.dev/) (only if C++20 is not available)
- [FreeType](https://www.freetype.org/)
- [GoogleTest](https://github.com/google/googletest)
- [libjpeg-turbo](https://www.libjpeg-turbo.org/)
- [libpng](http://www.libpng.org/pub/png/libpng.html)
- [md4c](https://github.com/mity/md4c)
- [zlib](https://www.zlib.net/)

Additional system dependencies on Linux:

- [Fontconfig](https://www.fontconfig.org)
- [GDK3](https://developer.gnome.org/gdk3/stable/) (3.22 or newer)
- [libepoxy](https://github.com/anholt/libepoxy)

### Compile and install

Fetch the repository including submodules:

    $ git clone --recursive --shallow-submodules https://github.com/cfillion/reaimgui.git

Build ReaImGui using Meson:

    $ meson setup build
    $ cd build
    $ ninja

The change the installation directory to a portable install:

    $ meson configure -Dresource_path=~/path/to/your/portable/install/

Downgrade to C++17 if compiling on macOS with Xcode older than 14:

    $ meson configure -Dcpp_std=c++17

Run the test suite using:

    $ meson test

Install ReaImGui into your REAPER resource path using:

    $ meson install --tags runtime

### Cross-compilation

#### Linux

QEMU is used to run compiled binaries for binding generation and tests.

    meson setup build/i686    --cross-file cross/i686-linux-gnu.ini
    meson setup build/armv7l  --cross-file cross/arm-linux-gnueabihf.ini
    meson setup build/aarch64 --cross-file cross/aarch64-linux-gnu.ini

#### macOS

macOS 10.14 (or older) and Xcode 9 are required for producing 32-bit builds.
Xcode 12 is required to compile for ARM64.

    export MACOSX_DEPLOYMENT_TARGET=10.9
    meson setup build/i386 --cross-file cross/i386-darwin.ini

    export MACOSX_DEPLOYMENT_TARGET=11.0
    meson setup build/arm64 --cross-file cross/arm64-darwin.ini
