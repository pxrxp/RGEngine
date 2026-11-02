# Project structure

```text
CMakeLists.txt        top-level project (RGEngine), options, sanitizers, vcpkg,
                      install and CPack rules
cmake/                toolchain-mingw.cmake: Windows cross-compilation
vcpkg.json            dependencies and GLFW backend features
build.sh / build.bat  configure + build wrappers
run.sh / run.bat      run the executable (optionally under a debugger)
engine/
  CMakeLists.txt      static library `engine` (globs src/*.cpp)
  include/engine/     public headers
    components/       TransformComponent, PhysicsComponent, BoundingBox, vertex formats
    repos/            RepoBase and the mesh/shader/texture/material repositories
    util/             filesystem, PNG, OBJ, GL debug, editor helpers
  src/                implementation, mirrors include/
  shaders/            AABB debug shader, skybox shader, postprocess vertex shader
game/
  CMakeLists.txt      executable `RivalGrounds`, links engine, stages assets
  include/rivalgrounds/  RivalGrounds.h (game class), Enemy.h
  src/                main.cpp, Enemy.cpp, and the RivalGrounds split into
                      RivalGrounds.cpp, PlayerController.cpp, Combat.cpp,
                      Hud.cpp and RivalGroundsDebug.cpp
  shaders/            cube.vert/frag (materials), grayscale.frag (post-process)
tests/                engine_tests (dependency-free harness, registered with CTest)
resources/            city.obj/mtl, Box.obj, M4Colt.obj, textures, skybox faces, logo
docs/                 engine/, game/, player-guide/, development/
```

## Layers

- **Engine** — window, input, camera, scene graph, renderer, resource repositories, collision, physics component, utilities. No game types or asset names.
- **Game** — `RivalGrounds` (a `Scene`), `Enemy`, the frame in `main.cpp`, the HUD, the debug menu, and the game's shaders.

Build targets:

- `engine` — static library, public includes `engine/include`, links glad/glfw/imgui/OpenGL/ZLIB/PNG.
- `RivalGrounds` — executable, includes `game/include`, links `engine`.
- `engine_tests` — test executable; compiles only the GL-free engine sources it needs and links `glm::glm`.

## Where code belongs

| Change | Location |
| --- | --- |
| New engine capability (window, renderer, repository) | `engine/` |
| Gameplay rules, content, HUD, input bindings | `game/` |
| Look of the game (material shader, post-process) | `game/shaders/` |
| Engine debug/generic shader | `engine/shaders/` |
