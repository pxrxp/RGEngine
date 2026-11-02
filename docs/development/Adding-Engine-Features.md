# Adding engine features

See [Extending the engine](../engine/Extending-the-Engine.md) for the boundary rules and step-by-step recipes. The short version:

1. **Identify the general capability.** "Post-process effect" is engine plumbing plus a game shader; "grayscale" is only a game shader. "Camera mouse look" is engine; "F1 menu" is game.
2. **Add it to `engine/`** with a header in `engine/include/engine/` and an implementation in the matching `engine/src/` path. `engine/CMakeLists.txt` globs sources, so no build edit is needed for a new `.cpp`.
3. **Respect the boundary**: no `#include <rivalgrounds/...>`, no asset names, no gameplay constants, no window titles.
4. **Add a hook rather than a branch.** If the game needs to opt in, add a virtual method, a descriptor field, or a setter (`App::set_post_process` is the model).
5. **Cached or owned resources** go through a `RepoBase` subclass and are cleared in `App::cleanup()`.
6. **Update the docs** in `docs/engine/` and run a clean build with `ENABLE_WERROR=ON`.

## Checklist for a new engine API

- [ ] Public header documents ownership and failure mode.
- [ ] Loader returns empty on failure and logs a useful message.
- [ ] GL objects are released in a destructor.
- [ ] The game compiles without changes, or the game's use is updated in the same commit.
- [ ] `docs/engine/` describes the new API.
