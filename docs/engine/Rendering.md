# Rendering

Rendering is split into staging and drawing: the scene builds render packets, `Renderer` draws them.

## Submission

```cpp
scene->submit_to_renderer();          // Renderer::submit(root)
app.get_renderer().render();
```

`Renderer::submit()` walks the scene tree from the root's children, adds each visible packet once (a node can only be reached once through the parent links), and stores the packets in a queue.

`Renderer::render()` sorts the queue by:

1. priority (`HIGH` → `NORMAL` → `LOW`),
2. shader program id,
3. material id.

It then binds a shader program only when it changes, binds/unbinds materials around their draw calls, invokes each packet's `render` lambda, and clears the queue. Sorting by program and material keeps GPU state changes low; grouping by material means a material is bound once per contiguous run.

## Where the frame is drawn

`App::begin_scene_render()` renders into an offscreen framebuffer (see [Application](Application.md)); `App::end_scene_render()` resolves MSAA and presents:

- with a post-process program: fullscreen triangle sampling `u_scene` and `u_scene_aux`;
- without one: a `glBlitFramebuffer` of the resolved colour into the current draw framebuffer.

Both paths save and restore the previous framebuffer, viewport, program, VAO, active texture unit, texture bindings, and depth/blend/scissor/cull/colour-mask state.

## Materials and shaders

A material binds its uniforms and its textures (see [Textures and materials](Textures-and-Materials.md)). The material system owns a global texture-slot table sized by `GL_MAX_TEXTURE_IMAGE_UNITS`, so each texture name receives a unit while it is bound and releases it on unbind.

## Engine-provided shaders

| Shader | Use |
| --- | --- |
| `AABB.vert/frag/geo` | Thick debug lines; used by `Scene::debug_camera()` for the frustum visualiser. |
| `skybox.vert/frag` | Cube-mapped skybox with `gl_Position.xyww` and `LEQUAL` depth. |
| `postprocess.vert` | Fullscreen triangle for the post-process contract. |

Game shaders live in `game/shaders` and are staged next to the executable together with the engine shaders.

## ImGui

`App::render_debug_gui()` draws ImGui on top of the resolved frame. Games commonly draw HUD elements through `ImGui::GetForegroundDrawList()` inside their own scene submission.

## No built-in lighting or shadowing

Lighting is a property of the shader a material uses. The engine ships no lit shader; RivalGrounds' `cube.frag` implements its own directional light, ambient hemisphere and gamma correction.
