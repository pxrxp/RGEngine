# RivalGrounds

RivalGrounds is a C++20 / OpenGL 3.3 project made of a small generic engine and a first-person game built on top of it.

- `engine/` is the reusable layer: window and input, camera, scene graph, renderer, resource repositories, collision and physics components, asset utilities, and generic shaders. It contains no game content.
- `game/` is RivalGrounds: a city level, a player controller, a hitscan rifle, enemy AI, an objective, a HUD, and the game's own shaders (material and post-process).

## Build

The project is built with CMake and vcpkg; dependencies are declared in `vcpkg.json`. Convenience wrappers:

- `build.sh` / `build.bat` — configure and build (requires `TRIPLET`, `BUILD_TYPE`, `ENABLE_ASAN_UBSAN`, `ENABLE_WERROR`, `GLFW_BACKEND`)
- `run.sh` / `run.bat` — run the built executable

`cmake --build build` also stages `shaders/` and `resources/` next to the executable, because assets are resolved relative to the binary.

See `docs/development/Building.md` and `docs/development/Dependencies.md` for details.

## Documentation

Start at [`docs/README.md`](docs/README.md):

- `docs/engine/` — engine systems and extension guide
- `docs/game/` — game loop, player, weapons, enemies, world, HUD, debug tools
- `docs/player-guide/` — install, controls, gameplay, troubleshooting
- `docs/development/` — building, structure, conventions, debugging, limitations

## License

MIT. See `LICENSE` and `LICENSES.md`.
