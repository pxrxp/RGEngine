# Application

`App` (`engine/include/engine/App.h`, `engine/src/App.cpp`) is the engine's runtime shell.

## Configuration

```cpp
struct AppConfig
{
  std::string window_title = "Application";
  int width = 0;          // 0 = half of the primary monitor
  int height = 0;
  bool decorated = true;
  int msaa_samples = 4;
};

App app(AppConfig{ .window_title = "My Game" });
app.init();
```

`init()` creates a GLFW window with an OpenGL 3.3 core profile and a debug context, loads glad, installs the GL debug callback, enables multisampling and depth testing, creates the fullscreen-triangle VAO used by the post-process pass, sets up ImGui, and initialises the material repository's texture-slot table.

`init()` does not decide the cursor mode and does not set a window title other than the configured one. Input policy such as "capture the mouse for first-person play" belongs to the game.

## Frame API

| Call | Meaning |
| --- | --- |
| `is_running()` | `true` while the window is open. |
| `update()` | Poll events, mouse look (only when the cursor is `GLFW_CURSOR_DISABLED`), reset mouse delta, update delta time. |
| `init_debug_gui()` | Start an ImGui frame. Despite the name it is also the normal UI frame start. |
| `begin_scene_render()` | Bind the offscreen scene target and clear the auxiliary attachment. |
| `end_scene_render()` | Resolve MSAA, run the post-process program if one is set, restore GL state and the previous viewport/framebuffer. |
| `render_debug_gui()` | Render ImGui draw data with blending enabled and depth testing disabled. |
| `cleanup()` | Clear the renderer and all resource repositories. |

## Offscreen target

The scene is rendered into two multisampled colour attachments plus a depth attachment:

- attachment 0 (`RGBA8`) is the scene colour;
- attachment 1 (`R8`) is a free auxiliary channel the game may write from its own fragment shaders (`layout(location = 1) out float ...`);
- `GL_DEPTH_COMPONENT24` for depth.

The target falls back to a single-sample allocation when the requested MSAA sample count cannot produce a complete framebuffer. If the framebuffer is still incomplete, rendering goes straight to the window.

## Post-process hook

```cpp
bool ok = app.set_post_process(ShaderProgramDescriptor{
  .program_name = "grayscale_postprocess",
  .shaders = { "postprocess.vert", "grayscale.frag" } });
```

The engine loads the program through `MaterialRepo::shader_program_repo` and binds `u_scene` to the resolved colour texture (unit 0) and `u_scene_aux` to the resolved auxiliary texture (unit 1). The shader is a fullscreen triangle driven by `postprocess.vert`. Without a program the resolved colour is blitted to the window.

## Owned resources

`App` exposes `mesh_repo` and `material_repo`. `MaterialRepo` in turn owns `tex_repo` and `shader_program_repo`, so all GPU resources reachable from a material share one repository tree. `App` does not own a second, unrelated texture cache.
