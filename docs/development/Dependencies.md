# Dependencies

Dependencies are declared in `vcpkg.json` and resolved by vcpkg during the first CMake configure.

| Package | Features | Used for |
| --- | --- | --- |
| `glad` | `extensions` | OpenGL 3.3 function loading |
| `glfw3` | platform backend (`x11`/`wayland`/`windows`) | Window, GL context, input |
| `opengl` | | Links the system GL library (`OpenGL::GL`) |
| `glm` | | Vector/matrix/quaternion math |
| `zlib` | | Required by libpng |
| `libpng` | | PNG decoding (`engine/util/png_image.cpp`) |
| `imgui` | `glfw-binding`, `opengl3-binding` | HUD and debug menu |

The manifest features `x11`, `wayland`, and `windows` enable the matching GLFW backend; `build.sh` passes `GLFW_BACKEND` as `VCPKG_MANIFEST_FEATURES`.

## CMake packages

```cmake
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
```

`engine` links `glad::glad`, `glfw`, `imgui::imgui`, `OpenGL::GL`, `ZLIB::ZLIB` and `PNG::PNG` as `PUBLIC` dependencies, so the game inherits them.

## Runtime

- OpenGL 3.3 core (the app requests a debug context and 4× MSAA).
- A display server on Linux (X11 or Wayland, matching the GLFW backend the project was built with).
- Assets next to the executable (`shaders/`, `resources/`), staged by the build.
