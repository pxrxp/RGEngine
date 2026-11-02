# RGEngine documentation

This is the index for the documentation. If code and documentation disagree, the code is correct — please fix the page.

## Sections

| Section | Contents |
| --- | --- |
| [Engine](engine/Home.md) | RGEngine: window, camera, scene, rendering, resources, collision, utilities |
| [Game](game/Home.md) | RivalGrounds: loop, player, weapon, enemies, world, HUD, debug tools |
| [Player guide](player-guide/Home.md) | Install, controls, gameplay, combat, enemies, troubleshooting |
| [Development](development/Home.md) | Building, dependencies, structure, conventions, debugging, extending |

## Repository map

```text
CMakeLists.txt          top-level build (C++20, warnings as errors, sanitizers)
vcpkg.json              dependency manifest
build.sh / build.bat    configure + build wrappers (staged via CMake)
run.sh / run.bat        run the executable
cmake/                  cross-compilation toolchains (llvm-mingw)
engine/                 RGEngine: the engine static library and engine shaders
game/                   RivalGrounds: the game and game shaders
resources/              models, textures, skybox faces
docs/                   this documentation
```

## Quick facts

- RGEngine is the project; RivalGrounds is its game. C++20, OpenGL 3.3 core, GLFW, glad, glm, Dear ImGui, libpng, zlib, managed with vcpkg.
- `engine/` never references game types, game assets, or game content.
- Assets are loaded relative to the executable; the build stages `shaders/` and `resources/` next to `RivalGrounds`.
- The game is a first-person shooter with one hitscan weapon, one map, patrol/combat enemy AI, and a scroll objective.
