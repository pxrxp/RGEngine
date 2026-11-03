<p align="center">
<img width="433" height="68" alt="image" src="https://github.com/user-attachments/assets/ab6140f1-79df-4beb-bda0-c8dacd10a680" />
</p>

![C++](https://img.shields.io/badge/c%2B%2B-%2300599C.svg?style=for-the-badge&logo=cplusplus&logoColor=white)
![OpenGL](https://img.shields.io/badge/opengl-%235586A4.svg?style=for-the-badge&logo=opengl&logoColor=white)

A small 3D engine in C++20 and OpenGL 3.3, and RivalGrounds, a first-person
shooter written using the engine.

`engine/` handles the window, the renderer, the scene graph, collision and the
asset loaders. `game/` holds the city, the player, the enemies and the rules, and
only uses the engine's public headers. To build something else on the engine,
`game/` is the part you replace.

## The game

https://github.com/user-attachments/assets/3d660579-2300-4d22-b59b-2b279ec75bf8

You come down over an abandoned city on another world. Your spaceship doesn't make
it, and the scroll you were sent for is somewhere in the streets below. The
people already living there don't want you around.

The city is lifeless; the things that matter keep their color — enemies, your
rifle, the scroll. Soldiers patrol, notice you, and shoot. Your ship scattered
crates across the city as it broke up, and you can pick them up and stack them,
which is how you reach anything the scroll landed on top of.

A match starts on the title screen, drops you somewhere random with ten seconds
of grace, and ends when you touch the scroll or run out of health.

## What the engine has

- GLFW window, input polling, cursor capture
- Camera with view and projection helpers
- Scene graph with cached world-space bounding boxes
- Forward renderer with an MSAA framebuffer and a pluggable post-process pass
- Cached repositories for meshes, shaders, materials and textures
- OBJ and PNG loading, skybox rendering
- Triangle-mesh collider built from a BVH, plus ray and overlap queries
- Dear ImGui for debug UI

The postprocessing pass, the HUD, the weapon and everything else in RivalGrounds live
in `game/`, not in the engine.

## Requirements

- CMake 3.15+, a C++20 compiler
- [vcpkg](https://github.com/microsoft/vcpkg) in `vcpkg/` — the build scripts
  clone and bootstrap it if it is missing. Dependencies are in `vcpkg.json`
  (glad, GLFW, glm, Dear ImGui, libpng, zlib)
- Linux: the X11 and Wayland development headers GLFW builds against, and a
  driver that can create an OpenGL 3.3 core context
- Windows: MinGW and a `x64-mingw-dynamic` triplet

## Build

`build.sh` and `build.bat` read these variables:

| Variable | Example | Meaning |
| --- | --- | --- |
| `TRIPLET` | `x64-linux`, `x64-mingw-dynamic` | vcpkg triplet |
| `BUILD_TYPE` | `Debug`, `Release` | CMake build type |
| `ENABLE_ASAN_UBSAN` | `ON`, `OFF` | Address and UB sanitizers |
| `ENABLE_WERROR` | `ON`, `OFF` | Warnings as errors |
| `GLFW_BACKEND` | `wayland`, `x11`, `windows` | GLFW backend (Unix); `wayland` covers both Wayland and X11 at runtime |

Linux:

```bash
export TRIPLET=x64-linux
export BUILD_TYPE=Release
export ENABLE_ASAN_UBSAN=OFF
export ENABLE_WERROR=ON
export GLFW_BACKEND=wayland
./build.sh          # ./build.sh -c to drop the CMake cache first
./run.sh
```

Windows:

```bat
set TRIPLET=x64-mingw-dynamic
set BUILD_TYPE=Release
set ENABLE_ASAN_UBSAN=OFF
set ENABLE_WERROR=ON
build.bat
run.bat
```

Or call CMake yourself:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_ASAN_UBSAN=OFF -DENABLE_WERROR=ON \
  -DVCPKG_TARGET_TRIPLET=x64-linux -DVCPKG_HOST_TRIPLET=x64-linux \
  -DVCPKG_MANIFEST_FEATURES=wayland
cmake --build build --parallel
```

Shaders and assets are looked up relative to the executable, so a post-build
step copies `engine/shaders/`, `game/shaders/` and `resources/` next to
`build/game/RivalGrounds`. No manual copy is needed.

## Controls

| Input | Action |
| --- | --- |
| `WASD` / arrows | Move |
| `Ctrl` | Sprint |
| `Shift` | Sneak |
| `Space` | Jump |
| Left click | Fire |
| Right click | Grab a crate, or throw the held one |
| `H` | Show or hide the crate you are holding |
| `Tab` / `Esc` | Capture or release the mouse |
| `R` | Restart |
| `F1` | Debug menu |

## Tests

```bash
ctest --test-dir build --output-on-failure
```

`tests/` builds `engine_tests`, which covers the bounding-box and triangle-mesh
collider maths. It links only the parts of the engine that don't need a GL
context, so it runs headless.

## Releases

```bash
cmake --build build --target package
```

CPack writes `build/RGEngine-RivalGrounds-<version>-<system>-<arch>.zip`
containing:

```text
RivalGrounds(.exe)
shaders/      engine/shaders + game/shaders, merged
resources/    models, textures, skybox, logo (~76 MB)
LICENSE
LICENSES.md
```

The Windows archive links the MinGW runtime statically and imports only system
DLLs. The Linux archive still needs the system OpenGL, X11/Wayland and GLFW
libraries, as any desktop OpenGL program does.

Windows archives can be cross-built from Linux with llvm-mingw:

```bash
export LLVM_MINGW_ROOT=/path/to/llvm-mingw-<date>-ucrt-ubuntu-22.04-x86_64
export PATH="$LLVM_MINGW_ROOT/bin:$PATH"
cmake -B build-win -S . \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$PWD/cmake/toolchain-mingw.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-static -DVCPKG_HOST_TRIPLET=x64-linux \
  -DCMAKE_BUILD_TYPE=Release -DENABLE_ASAN_UBSAN=OFF -DENABLE_WERROR=ON
cmake --build build-win --parallel
cmake --build build-win --target package
```

Last built here: Linux x86_64 with GCC (build, tests, package; archive extracted
and run) and Windows x86_64 with llvm-mingw 20261020 (cross build and package;
not executed, there was no Windows host or Wine available to try it).

## Layout

```text
CMakeLists.txt   project, warnings, sanitizers, install/CPack rules
vcpkg.json       dependencies and GLFW backend features
build.sh/.bat    configure and build wrappers
run.sh/.bat      run the executable, optionally under a debugger
engine/          RGEngine, a static library, GLSL in engine/shaders/
game/            RivalGrounds, the executable, GLSL in game/shaders/
tests/           headless engine tests
resources/       city, crates, weapon, textures, skybox, logo
docs/            engine, game, player guide and development docs
```

## Documentation

Start at [`docs/README.md`](docs/README.md).

- [`docs/engine/`](docs/engine/Home.md) — the engine's systems and how to extend them
- [`docs/game/`](docs/game/Home.md) — RivalGrounds: game loop, player, combat, enemies, world
- [`docs/player-guide/`](docs/player-guide/Home.md) — installing and playing
- [`docs/development/`](docs/development/Home.md) — building, structure, conventions, debugging

Where a page and the source disagree, the source is right.

## License

MIT, see [`LICENSE`](LICENSE). Third-party credits are in
[`LICENSES.md`](LICENSES.md).
