# Troubleshooting

## Build fails immediately

- `build.sh` exits with "environment variable not set": export `TRIPLET`, `BUILD_TYPE`, `ENABLE_ASAN_UBSAN`, `ENABLE_WERROR`, and `GLFW_BACKEND` first.
- vcpkg errors on first configure usually mean the submodule/toolchain is missing or the network is unavailable. Delete `build/CMakeCache.txt` and reconfigure.
- `-Werror` is on by default (`ENABLE_WERROR=ON`). A new compiler version may turn a previous warning into an error; fix the warning rather than disabling it project-wide in the long term.

## Build succeeds but the game exits at startup

- "Failed to initialize GLFW" — no display server available (headless shell, no X11/Wayland session).
- "Failed to create GLFW window" — the requested OpenGL 3.3 core context is not available; check the driver.
- "Failed to initialize GLAD" — the GL loader could not resolve functions; usually the context was not created.

## The window opens but the screen is black or the world is missing

- Shader compile/link errors are printed to the console with their info logs.
- Missing textures print `ERROR: Invalid file ...` and the material simply lacks that texture.
- `Rival Grounds: city.obj has no triangle geometry; city not spawned.` means the model failed to parse.
- Make sure `shaders/` and `resources/` exist next to the executable; a CMake build stages them automatically.

## The mouse does not look around

The cursor must be captured: `Tab` captures it and `Escape` releases it. Closing the F1 debug menu also captures it.

## The game is very slow

Debug builds enable AddressSanitizer and UndefinedBehaviorSanitizer by default. Reconfigure with `ENABLE_ASAN_UBSAN=OFF` and `BUILD_TYPE=Release` for normal performance.

## Where the game writes output

Everything goes to stdout/stderr: GL debug messages, shader logs, missing-asset warnings, the city collision triangle count, the enemy spawn count and nearest distance, and the MSAA line. Copy that output when reporting a problem.
