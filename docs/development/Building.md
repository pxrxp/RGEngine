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
