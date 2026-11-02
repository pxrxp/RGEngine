# Utilities

## Filesystem (`engine/include/engine/util/filesystem.h`)

- `Util::get_exe_dir()` — directory of the running executable (`/proc/self/exe` on Linux, `_NSGetExecutablePath` on macOS, `GetModuleFileNameA` on Windows).
- `Util::read_file(relative_path)` — string contents of `<exe_dir>/<relative_path>`; throws `std::runtime_error` when the file cannot be opened.
- `Util::read_file_bytes(relative_path)` — same, as a byte vector.

All asset paths in the engine are relative to the executable, not the working directory. Builds stage `shaders/` and `resources/` next to the binary.

## PNG loading (`util/png_image.h`)

`PNGImage(relative_path)` reads `resources/<relative_path>` with libpng through an in-memory read callback. It strips 16-bit data, expands palettes, tRNS, and sub-8-bit greyscale, adds an alpha channel when missing, converts greyscale to RGB, and exposes `width`, `height`, `channels`, `pixels`, and `is_valid()`. Invalid files log to stderr and leave `is_valid() == false`.

## OBJ loading (`util/obj_model.h`)

- `from_OBJ<Vertex, Indices>(path, draw_primitive, usage)` — parses `v`, `vt`, `vn`, `f`; supports positive and negative indices, triangles, quads and n-gons (fan triangulation); deduplicates vertices per position/UV/normal key; flips V; returns a `MeshDescriptor`. The file is read from `resources/<path>`.
- `from_OBJ_material_groups<Vertex, Indices>(path, ...)` — groups faces by `usemtl`, reads `mtllib` files (`Kd`, `d`/`Tr`, `map_Kd` with basic option stripping), resolves texture paths, and returns one `ObjMaterialMeshGroup` per material with its own mesh descriptor, diffuse colour, opacity and texture path.

## GL debug output (`util/gl_debug.h`)

`Util::init_gl_debug()` enables `GL_KHR_debug` (or `GL_ARB_debug_output`) when the context has the debug bit. Notifications and four common driver ids are filtered. `App::init()` calls it after loading glad.

## Debug helpers (`debug.h`)

Debug-only `operator<<` for `std::vector<T>` plus glm's IO operators, used by the `Camera`, `Mesh`, and `PhysicsComponent` debug printers.

## Definitions (`util/definitions.h`)

Minimal GL typedefs, the `RGBA_BLACK` colour instance, axis constants (`AXIS_X`, `AXIS_Y`, `AXIS_NEG_Z`), and cube vertex/face/edge macros used by the skybox and the debug frustum.

## OpenGL include (`util/opengl.h`)

Includes glad and GLFW so engine headers get GL types and window functions from one place.
