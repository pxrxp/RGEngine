# Extending the engine

## Rules of the boundary

1. The engine must not include `<rivalgrounds/...>` or mention game types, asset names, window titles, or gameplay values.
2. Game behaviour belongs in `game/`: player, enemies, HUD, world content, post-process look, input bindings.
3. Generic capability belongs in `engine/`: rendering plumbing, resource loading, math, collision queries, windowing, debug tooling.
4. When a game needs a hook, add a general hook, not a game-specific branch. `Scene`'s collision virtuals and `App::set_post_process()` exist for that reason.

## Adding a resource type

1. Add `Resource` class in `engine/include/engine/<Name>.h` with an explicit destructor for its GL objects and deleted copy operations.
2. Add a descriptor plus `<Name>Repo : RepoBase<Key, Resource>` in `engine/include/engine/repos/`.
3. Implement loading in `engine/src/repos/<Name>Repo.cpp`, returning `{}` and logging on failure.
4. Wire the repository into `App` (or into `MaterialRepo` if it is material-adjacent) and clear it in `App::cleanup()`.
5. Document the loader in `Resource-Repositories.md`.

## Adding a vertex format

1. Add field macros and a `DEFINE_VERTEX_STRUCT` entry in `components/vertex_formats.h`.
2. Add the type to the `static_assert` lists in `MeshRepo::load_mesh` and `obj_model.h`.
3. Add a `VERTEX_*_FIELDS(SETUP_ATTRIB)` branch in `MeshRepo::load_mesh`.

## Adding an engine shader

Put it in `engine/shaders` only if the engine itself uses it. Shaders that define the game's look go in `game/shaders`. Both directories are staged by CMake.

## Adding a post-process effect

Write a fragment shader that declares `in vec2 v_uv`, `uniform sampler2D u_scene` and optionally `uniform sampler2D u_scene_aux`, and return it from the game:

```cpp
app.set_post_process(ShaderProgramDescriptor{
  .program_name = "my_effect",
  .shaders = { "postprocess.vert", "my_effect.frag" } });
```

Fragment shaders in the scene pass can write the auxiliary channel with `layout(location = 1) out float ...;`.

## Adding collision geometry

- Mesh geometry: `collision_world.add_static_mesh(mesh->get_collision_triangles(), world_matrix)`.
- Scene-object AABBs: mark the object `set_collidable(true)` and let `Scene::collect_colliders_recursive` pick it up.
- Forward `Scene`'s collision hooks to whatever owns the data (see RivalGrounds' overrides).

## Adding configurable app behaviour

Extend `AppConfig` for values the engine owns at startup (window size, samples, title, decoration). Do not add gameplay toggles there; pass them to a `Scene` subclass instead.
