# Development

- [Building](Building.md) — toolchain, CMake options, scripts, asset staging
- [Dependencies](Dependencies.md) — what vcpkg installs and why
- [Project structure](Project-Structure.md) — files and layers
- [Coding conventions](Coding-Conventions.md) — the style actually used in the sources
- [Debugging](Debugging.md) — logs, sanitizers, debugger, F1 menu
- [Adding game features](Adding-Game-Features.md)
- [Adding engine features](Adding-Engine-Features.md)
- [Known limitations](Known-Limitations.md) — what the project does not do

## The one boundary rule

`engine/` must stay free of game content, and `game/` may use all of `engine/`. If a change needs game knowledge in the engine, add a general hook instead (see [Extending the engine](../engine/Extending-the-Engine.md)).
