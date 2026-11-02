# Installation

RivalGrounds is built from source with CMake and vcpkg. Release archives (executable, shaders and assets in one zip) can be produced with `cmake --build build --target package`; there is no platform installer.

## Requirements

- A C++20 compiler (GCC or Clang on Linux/macOS, MinGW on Windows)
- CMake 3.15 or newer
- Network access on the first configure so vcpkg can fetch dependencies
- An OpenGL 3.3-capable GPU and driver, plus X11 or Wayland on Linux

## Build with the scripts

`build.sh` and `build.bat` require these environment variables:

```bash
export TRIPLET=x64-linux          # or x64-mingw-dynamic on Windows
export BUILD_TYPE=Debug           # Debug or Release
export ENABLE_ASAN_UBSAN=ON       # OFF for a release run
export ENABLE_WERROR=ON
export GLFW_BACKEND=wayland       # wayland | x11 | windows (wayland also runs on X11)

./build.sh            # add -c to clear the CMake cache first
```

The first build bootstraps vcpkg and builds the dependencies from `vcpkg.json`, so it takes a while. Later builds reuse `build/`.

## Build manually

```bash
cmake -B build -S .   -DCMAKE_BUILD_TYPE=Debug   -DENABLE_ASAN_UBSAN=OFF   -DENABLE_WERROR=ON   -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake   -DVCPKG_TARGET_TRIPLET=x64-linux   -DVCPKG_MANIFEST_FEATURES=x11
cmake --build build --parallel
```

A CMake post-build step copies `engine/shaders` and `game/shaders` into `build/game/shaders` and `resources` into `build/game/resources`.

## Run

```bash
./run.sh          # run build/game/RivalGrounds
./run.sh -g       # under gdb
./run.sh -l       # under lldb
```

`run.sh` builds first if the binary is missing. If you build with sanitizers enabled and the game is slow or the debugger misbehaves, rebuild with `ENABLE_ASAN_UBSAN=OFF`.

## Notes

- Debug builds include the F1 debug menu. Release builds remove it.
- Assets are resolved relative to the executable, not the working directory, so the built game runs from anywhere as long as `shaders/` and `resources/` sit next to the binary (the build stages them there).
