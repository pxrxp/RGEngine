# Contributing

RGEngine is a small C++20/OpenGL engine with RivalGrounds as its example game.
A focused bug fix, test or documentation correction is more useful than a
large change without a clear reason.

## Before changing code

- For a bug, include what you did, what happened and what you expected.
- For a larger change, open an issue first so the scope can be discussed.
- Check `docs/development/` before changing engine or game structure.
- For security issues, follow [`SECURITY.md`](SECURITY.md); do not open a
  public issue or pull request with vulnerability details.

## Keep the boundary

`engine/` is general-purpose and must not depend on game types, assets or
content. `game/` can use the engine's public headers. If a feature needs game
knowledge in the engine, add a general hook rather than a game-specific branch.

## Code and tests

Follow [the coding conventions](docs/development/Coding-Conventions.md). In
particular, use the repository's `.clang-format` file and keep the existing
naming and file layout.

Add or update tests when the change affects testable engine behaviour. The
current tests cover bounding boxes and triangle-mesh collider maths and run
without a GL context.

Build and run the tests before submitting:

```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The build scripts require `TRIPLET`, `BUILD_TYPE`, `ENABLE_ASAN_UBSAN`,
`ENABLE_WERROR` and `GLFW_BACKEND`; see the README for examples. A clean build
with `ENABLE_WERROR=ON` is useful for catching warnings. If you cannot build
or test a change, say what you tried and where it stopped.

## Pull requests

Keep a pull request about one change. Include:

- What changed and why.
- How you built and tested it, including anything you could not test.
- Screenshots or a short recording for visible game or renderer changes.

Update the relevant docs when behaviour, build steps or public engine APIs
change. Do not include generated build output or unrelated formatting changes.

By submitting a contribution, you confirm you have the right to submit it and
that it can be distributed under the project's MIT license. Keep third-party
code and assets' license and attribution requirements intact.
