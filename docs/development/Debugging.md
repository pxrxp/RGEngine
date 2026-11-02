# Debugging

## First place to look: stdout/stderr

The game logs:

- GL debug output (`Util::init_gl_debug`, KHR_debug or ARB_debug_output) with source, type, and severity;
- shader compile and program link info logs on failure;
- missing shader, texture, and mesh errors;
- `Rival Grounds: city mesh loaded at scale ... (N collision triangles); boundary walls added.`;
- `Scene antialiasing: Nx MSAA` or the single-sample fallback.

The engine also prints resource destruction messages in debug builds (`Deleting shader ...`, `Destroying Mesh ID ...`), which is useful for spotting lifetime bugs.

## Sanitizers

`ENABLE_ASAN_UBSAN` defaults to `ON`. AddressSanitizer catches out-of-bounds and use-after-free, UndefinedBehaviorSanitizer catches invalid arithmetic and casts. Sanitizers complicate debugger stepping; disable them while stepping with a debugger:

```bash
cmake -B build -S . -DENABLE_ASAN_UBSAN=OFF -DENABLE_WERROR=ON
```

`ENABLE_TSAN` exists as an alternative when ASan/UBSan are disabled, but the engine does not create threads itself.

## Debugger

```bash
./run.sh -g    # gdb
./run.sh -l    # lldb
```

Debug builds use `-g3` for macro information. `ENABLE_WERROR` keeps the build warning-free, which is worth preserving.

## Runtime inspection

Debug builds expose the F1 menu: player position/health/grounded state, teleport, noclip, invincibility, hitbox visualisation, camera controls, physics counts, and an enemy table. See [Debug tools](../game/Debug-Tools.md).

## Common failure patterns

| Symptom | Likely cause |
| --- | --- |
| Uniform warning `not found in program` | Shader uniform was optimised out or the name has a typo |
| `Material::bind(): ... has no loaded texture named ...` | Material descriptor references a texture that failed to load or is not in `texture_desc_list` |
| `Material ... already exists` no longer prints | Intended: cached loads are silent |
| Black screen with a valid program | Scene framebuffer incomplete, shader error, or camera inside geometry |
| City missing | `city.obj` failed to load or has no triangles; check the console |
| Textures missing only in release | Assets not staged next to the executable |
