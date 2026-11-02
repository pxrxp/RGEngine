# Scene system

`Scene` (`engine/include/engine/Scene.h`) is the base class a game derives from. It owns a root `SceneObject` and provides the shared scene services.

## Construction and lifecycle

```cpp
class MyGame : public Scene
{
public:
  explicit MyGame(App& app) : Scene(app) {}
  void init() override { /* build the object tree */ }
  void update_physics() override { Scene::update_physics(); /* game update */ }
};

auto scene = std::make_shared<MyGame>(app);
scene->init();
```

- `Scene::init()` is an empty virtual hook; games build their world there.
- `submit_to_renderer()` pushes the object tree into `App::get_renderer()`.
- `get_app_cache()` returns the `App&`, which is how scenes reach repositories, the camera, and the window.
- `get_scene_ptr()` returns the root object.

## Fixed-step update

`update_physics()` accumulates `App::get_delta_time()` (clamped to 0.2 s) and calls `step_simulation()` once per `Scene::FIXED_STEP` (1/60 s). `step_simulation()` is virtual; the base implementation:

1. collects every enabled `PhysicsComponent` in the tree;
2. clears forces and applies gravity (`-14 m/s²` on the Y axis) to gravity-enabled dynamic objects;
3. damps momentum, integrates velocity and position;
4. resolves dynamic-vs-dynamic stacking with a spatial hash in the XZ plane;
5. settles gravity objects onto `find_ground_surface()`.

## Collision and uniform hooks

`Scene` exposes non-virtual entry points over protected virtual hooks so game code and engine helpers can call them:

| Public | Protected virtual |
| --- | --- |
| `blocks_vertical_capsule_for_actor(...)` | `blocks_vertical_capsule(...)` (default `false`) |
| `raycast_static_geometry(...)` | `static_mesh_ray_hit(...)` (default `false`) |
| `find_actor_ground_surface(...)` | `find_ground_surface_for_actor(...)` (defaults to `find_ground_surface`) |
| — | `find_ground_surface(...)` (default `false`) |
| — | `find_landing_surface(...)` (defaults to `find_ground_surface`) |

A game that owns a `CollisionWorld` forwards these hooks to it, as RivalGrounds does.

## Render helpers

`set_view_matrix(packet)`, `set_projection_matrix(packet)` and `set_model_matrix(packet, model)` set the usual `u_view`, `u_proj` and `u_model` uniforms. `set_view_matrix` and `set_projection_matrix` accept an optional uniform name and explicit matrix; the default argument queries the camera.

## Other services

- `capture_weak(object, fn)` produces a render callback that holds a weak reference to a scene object, checks `visible`, and passes `(object, render_packet)` when both still exist. Use it for every `set_render_packet` call that needs a lambda.
- `collect_colliders_recursive(object, out)` appends the world AABBs of collidable objects that have a render packet.
- `get_all_physics_objects()` returns every object with an enabled `PhysicsComponent`.
- `setup_skybox(object, texture_name)` configures a cube-mapped skybox with depth function `LEQUAL`.
- `debug_camera()` (debug builds only) adds an ImGui camera panel and an optional frustum visualiser using the AABB shader.
