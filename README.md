# RGEngine

RGEngine is a small C++20 / OpenGL 3.3 engine, and this repository is its
reference game: **RivalGrounds**, a first-person shooter set in an abandoned
city.

The split is deliberate. `engine/` knows about windows, cameras, meshes and
shaders but nothing about cities, enemies or scrolls. `game/` owns every piece of
content and every gameplay rule, and talks to the engine only through public
headers. If you want to build something else on RGEngine, delete `game/` and
start there.

## What is in the box

| RGEngine | RivalGrounds |
| --- | --- |
| GLFW window, input polling, cursor modes | City level built from a scaled OBJ model plus invisible boundary walls |
| Camera with view/projection helpers | First-person controller with gravity, jumping, sprint, sneak, noclip (debug) |
| Scene graph with cached world AABBs | Hitscan rifle, viewmodel, recoil, tracers, hit markers |
| Forward renderer: MSAA framebuffer, colour + auxiliary attachments, pluggable post-process | Grayscale post-process that preserves colour on the things that matter |
| Cached repositories for meshes, shaders, materials and textures | 128 physics crates that can be shot, grabbed, thrown and stacked |
| OBJ and PNG loading, skybox | 14 patrolling enemies with vision, cover checks and reinforcements |
| BVH triangle-mesh collider, ray/AABB queries, capsule and ground tests | Scroll objective, win/lose states, HUD |
| Dear ImGui debug GUI and in-world editor helpers | F1 debug menu: teleport, noclip, invincibility, hitboxes, camera tuning |
| No game types, assets or content | All game types, assets and content |

## Requirements

- CMake 3.15 or newer, a C++20 compiler.
- [vcpkg](https://github.com/microsoft/vcpkg) at `vcpkg/`. The build scripts clone
  and bootstrap it if it is missing. Dependencies are declared in `vcpkg.json`:
  glad, GLFW, glm, Dear ImGui, libpng and zlib.
- On Linux, the dev packages GLFW needs to build against (X11 or Wayland
  development headers), plus a GPU/driver that can create an OpenGL 3.3 core
  context.
- On Windows, the scripts expect a MinGW triplet (`x64-mingw-dynamic`) and copy
  the required DLLs next to the executable.

## Build

The wrappers read a few environment variables:

| Variable | Example | Meaning |
| --- | --- | --- |
| `TRIPLET` | `x64-linux`, `x64-mingw-dynamic` | vcpkg triplet to build against |
| `BUILD_TYPE` | `Debug`, `Release` | CMake build type |
| `ENABLE_ASAN_UBSAN` | `ON`, `OFF` | Address/UB sanitizers (debug builds) |
| `ENABLE_WERROR` | `ON`, `OFF` | Treat warnings as errors |
| `GLFW_BACKEND` | `wayland`, `x11`, `windows` | GLFW backend feature (Unix only) |

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

Windows (from a shell with MinGW and CMake on `PATH`):

```bat
set TRIPLET=x64-mingw-dynamic
set BUILD_TYPE=Release
set ENABLE_ASAN_UBSAN=OFF
set ENABLE_WERROR=ON
build.bat
run.bat
```

Or drive CMake yourself:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_ASAN_UBSAN=OFF -DENABLE_WERROR=ON \
  -DVCPKG_TARGET_TRIPLET=x64-linux -DVCPKG_HOST_TRIPLET=x64-linux \
  -DVCPKG_MANIFEST_FEATURES=wayland
cmake --build build --parallel
```

Shaders and assets are resolved relative to the executable, so every build runs
a `POST_BUILD` step that copies `engine/shaders/`, `game/shaders/` and
`resources/` next to `build/game/RivalGrounds`. There is no manual copy step.

## Controls

| Input | Action |
| --- | --- |
| `WASD` / arrows | Move |
| `Ctrl` | Sprint |
| `Shift` | Sneak |
| `Space` | Jump |
| Left click | Fire |
| Right click | Grab a crate, or throw the held one |
| `H` | Show/hide the held crate (it stays held) |
| `Tab` / `Esc` | Capture / release the mouse cursor |
| `R` | Restart the match |
| `F1` | Debug menu |

## Tests

```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

`tests/` builds `engine_tests`, a dependency-free harness covering the
`BoundingBox` and `TriangleMeshCollider` math. Those tests compile only the
GL-free engine sources, so they run headless in CI.

## Releases

The install rules and CPack configuration ship a runnable archive, assets
included:

```bash
cmake --build build --target package
```

That produces `build/RGEngine-RivalGrounds-<version>-<system>-<arch>.zip`
containing:

```text
RivalGrounds(.exe)
shaders/      engine/shaders + game/shaders, merged
resources/    models, textures, skybox faces, logo (~76 MB)
LICENSE
LICENSES.md
```

Both archives are self-contained as far as the project is concerned. Windows
builds link the MinGW C++ runtime statically and import only system DLLs; Linux
builds expect the system OpenGL, X11/Wayland and GLFW runtime libraries, which is
normal for a desktop OpenGL application.

A Windows archive can also be produced from Linux with llvm-mingw and the
vcpkg `x64-mingw-static` triplet:

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

| Verified build | Result |
| --- | --- |
| Linux x86_64, GCC, vcpkg static libraries | build, tests, package; archive ran from an extracted directory |
| Windows x86_64, llvm-mingw 20261020 + `x64-mingw-static` | cross build and package; imports only system DLLs (not executed: no Windows host or Wine here) |

## Layout

```text
CMakeLists.txt   project, warnings, sanitizers, CPack/install config
vcpkg.json       dependency manifest (and GLFW backend features)
build.sh/.bat    configure + build wrappers
run.sh/.bat      run the executable, optionally under a debugger
engine/          RGEngine: static library, GLSL in engine/shaders/
game/            RivalGrounds: executable, GLSL in game/shaders/
tests/           headless engine tests (CTest)
resources/       city, crates, weapon, textures, skybox, logo
docs/            engine, game, player guide and development docs
```

## Documentation

Start at [`docs/README.md`](docs/README.md):

- [`docs/engine/`](docs/engine/Home.md) — RGEngine systems and how to extend them
- [`docs/game/`](docs/game/Home.md) — RivalGrounds: loop, player, combat, enemies, world
- [`docs/player-guide/`](docs/player-guide/Home.md) — install, controls, gameplay
- [`docs/development/`](docs/development/Home.md) — building, structure, conventions, debugging

The docs describe the code as it is, including its limits. If a page and the
source disagree, the source wins.

## License

MIT. See [`LICENSE`](LICENSE). Third-party components and asset credits are
listed in [`LICENSES.md`](LICENSES.md).
