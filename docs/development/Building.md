# Building

## Toolchain

- CMake 3.15+, C++20.
- vcpkg is expected at `vcpkg/` and is included from the top-level `CMakeLists.txt` via `vcpkg/scripts/buildsystems/vcpkg.cmake`. `build.sh`/`build.bat` clone and bootstrap it if the directory is missing.
- `glad`, `glfw3`, `imgui`, `OpenGL`, `ZLIB`, and `PNG` are found with `find_package`.

## CMake options

| Option | Default | Meaning |
| --- | --- | --- |
| `ENABLE_WERROR` | `ON` | Adds `-Werror` to `-Wall -Wextra -pedantic` |
| `ENABLE_ASAN_UBSAN` | `ON` | `-fsanitize=address,undefined` |
| `ENABLE_TSAN` | `OFF` | `-fsanitize=thread` (only if ASan/UBSan are off) |
| `USE_WAYLAND` | `ON` (Unix) | Selects the GLFW Wayland backend feature |
| `USE_X11` | `OFF` (Unix) | Selects the GLFW X11 backend feature |

Sanitizer flags are added globally (`add_compile_options`/`add_link_options`).

## Configuration and build

```bash
cmake -B build -S .   -DCMAKE_BUILD_TYPE=Debug   -DENABLE_ASAN_UBSAN=OFF   -DENABLE_WERROR=ON   -DVCPKG_TARGET_TRIPLET=x64-linux   -DVCPKG_HOST_TRIPLET=x64-linux   -DVCPKG_MANIFEST_FEATURES=x11
cmake --build build --parallel
```

`build.sh` wraps the same steps and requires `TRIPLET`, `BUILD_TYPE`, `ENABLE_ASAN_UBSAN`, `ENABLE_WERROR`, and `GLFW_BACKEND` in the environment. `-c` removes `build/CMakeCache.txt` and `build/CMakeFiles` before configuring.

## Asset staging

`game/CMakeLists.txt` has a `POST_BUILD` command that copies, relative to the executable:

```text
build/game/shaders/   <- engine/shaders then game/shaders
build/game/resources/ <- resources/
```

The engine resolves shaders from `shaders/<name>.glsl` and assets from `resources/<path>` relative to the executable directory (see `Util::get_exe_dir()`), so a CMake build is runnable without extra copying. There is no separate manual copy step in the build scripts anymore.

## Release archives

```bash
cmake --build build --target package
```

CPack writes `build/RGEngine-RivalGrounds-<version>-<system>-<arch>.zip` containing
the executable, the merged `shaders/` directory, `resources/`, `LICENSE` and
`LICENSES.md`. Assets are resolved relative to the executable, so the archive
runs as soon as it is extracted. Assets dominate the size (about 60 MB
compressed).

## Cross-compiling for Windows

`cmake/toolchain-mingw.cmake` builds a 64-bit Windows executable from Linux with
[llvm-mingw](https://github.com/mstorsjo/llvm-mingw) and the vcpkg
`x64-mingw-static` triplet:

```bash
export LLVM_MINGW_ROOT=/path/to/llvm-mingw-<date>-ucrt-ubuntu-22.04-x86_64
export PATH="$LLVM_MINGW_ROOT/bin:$PATH"
cmake -B build-win -S . \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$PWD/cmake/toolchain-mingw.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
  -DVCPKG_HOST_TRIPLET=x64-linux \
  -DCMAKE_BUILD_TYPE=Release -DENABLE_ASAN_UBSAN=OFF -DENABLE_WERROR=ON
cmake --build build-win --parallel
cmake --build build-win --target package
```

Notes:

- The toolchain file sets `CMAKE_FIND_ROOT_PATH` to the MinGW sysroot. Without it
  CMake finds the host's `libOpenGL.so` and the link fails.
- `MINGW` builds get `-static`, so the C++ runtime and unwinder are linked in.
  The executable then imports only Windows system DLLs (`kernel32`, `user32`,
  `gdi32`, `shell32`, `api-ms-win-crt-*`).
- Cross builds disable the vcpkg app-local step, which needs a Windows host to
  run PowerShell; the static triplet does not need it.
- Sanitizers are not available for the MinGW runtime, so keep
  `ENABLE_ASAN_UBSAN=OFF`.

Verified with llvm-mingw 20261020 (clang 23, libc++): configure, build and
package succeed and `llvm-objdump -p` shows no non-system imports. The resulting
`RivalGrounds.exe` was not executed here, since this machine has no Windows host
or Wine.

## Tests

```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

`tests/` builds `engine_tests`, a dependency-free harness covering the
`BoundingBox` and `TriangleMeshCollider` math that does not need a GL context.
There is no game-level test target yet.

## Running

```bash
./run.sh        # build/game/RivalGrounds
./run.sh -g     # gdb
./run.sh -l     # lldb
```

`run.bat` is the Windows equivalent.
