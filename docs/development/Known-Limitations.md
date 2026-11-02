# Known limitations

## Scope

- One map, one weapon, one enemy type, one objective.
- No audio, no menus beyond the intro/win/lose overlays, no settings screen, no save data.
- No networking, no multiplayer, no replay system.
- No asset pipeline: models and textures are loaded directly from `resources/`.

## Rendering

- No shadows, reflections, deferred shading, or post-processing other than the grayscale pass.
- No instanced rendering in the game (the engine's `Mesh::draw(instance_count)` supports it, but no caller uses it).
- No frustum culling: every render packet in the tree is submitted.
- `> 2` colour attachments, HDR, and explicit render-scale are not supported by `App`.

## Collision and physics

- Triangle geometry is static world collision only; dynamic bodies are AABBs settled by a simple stacking pass.
- Crates do not collide horizontally with the city mesh.
- Enemies do not collide with each other.
- No joints, constraints, friction model, or restitution.
- The player controller is kinematic (no momentum carry).

## World

- The city is one `city.obj`; there is no streaming or chunking. Everything is loaded once and stays loaded.
- Restarting a match rebuilds the scene tree; the parsed OBJ data is cached, but meshes/materials are reloaded through the repositories.

## Engine

- `Scene` is a single class with a fixed-step accumulator; there is one scene at a time.
- Input is polled; there are no input mappings or rebinding.
- No scripting, serialisation, or editor.
- The game's main class is a single large header (`RivalGrounds.h`), which slows incremental builds.

## Platform

- Build scripts exist for Linux/macOS (`build.sh`, `run.sh`) and Windows (`build.bat`, `run.bat`, MinGW via vcpkg).
- The engine's filesystem and GL includes have Windows, macOS, and Linux branches; other platforms are untested.

## Testing

- There is no automated test suite. Verification is build + manual play.
