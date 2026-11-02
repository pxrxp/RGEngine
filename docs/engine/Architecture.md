# Architecture

The engine is a compact C++20 / OpenGL 3.3 core layer. It does not use an ECS, an event bus, or a scheduler. Ownership is explicit: `App` owns the window, camera, renderer, and the shared material repository; a `Scene` owns its object tree; repositories own GPU resources.

## Core objects

| Object | Responsibility |
| --- | --- |
| `App` | Creates the window/GL context, ImGui, the offscreen scene target, and exposes window, camera, renderer, repos, delta time, and the frame entry points. |
| `Window` | GLFW window wrapper: polling (`is_key_pressed`, `is_mouse_button_pressed`), cursor mode, mouse delta, clear and swap. |
| `Camera` | Follows one scene object for position/orientation; builds view and projection matrices, applies mouse look, and exposes `look_at`. |
| `Scene` | Root `SceneObject`, fixed-step physics update, collision hooks, render submission, and uniform helpers. |
| `SceneObject` | Node in the scene tree with a `TransformComponent`, `PhysicsComponent`, optional render packet, visibility and collidability flags. |
| `Renderer` | Collects render packets from the scene, sorts them, binds materials, and invokes each packet's render lambda. |
| `Material`, `Mesh`, `Texture`, `Shader`, `ShaderProgram` | GPU resource wrappers with deterministic ownership and ids. |
| Repositories | Caches keyed by name (see [Resource repositories](Resource-Repositories.md)). |
| `CollisionWorld`, `TriangleMeshCollider` | Static triangle collision: BVH-accelerated capsule blocking, raycasts, ground and landing queries. |
| Components and utilities | Transform, physics, bounding box, OBJ/PNG loading, debug GL output, debug editor helpers. |

## Dependencies between layers

```text
game (RivalGrounds, Enemy, main.cpp, game shaders)
  └── engine (App, Scene, SceneObject, Renderer, repos, collision)
        └── glad, GLFW, glm, Dear ImGui, libpng, zlib
```

The engine never includes game headers and never references game types or asset names. Game code includes engine headers freely and is expected to drive the frame through `App`.

## Frame shape

1. `App::update()` polls events, updates mouse-look when the cursor is locked, resets the mouse delta, and updates delta time.
2. `App::begin_scene_render()` binds a multisampled offscreen target (colour + auxiliary attachment + depth) and clears the auxiliary channel.
3. The game submits its scene; `Renderer::render()` sorts and draws.
4. `App::end_scene_render()` resolves the target and either runs the game's post-process program or blits the result to the window.
5. The game draws UI through ImGui, then `Window::swap_buffers()`.

Game logic itself lives in a `Scene` subclass; the engine has no gameplay update of its own.
