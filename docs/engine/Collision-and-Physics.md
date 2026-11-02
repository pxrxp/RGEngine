# Collision and physics

The engine ships collision queries and a simple physics component. It is not a rigid-body engine: dynamic objects respond to gravity, stacking and ground settling, but triangles act as world geometry rather than as objects that react to dynamic bodies.

## BoundingBox

`BoundingBox` (`engine/include/engine/components/BoundingBox.h`) stores `m_min`/`m_max`, supports `add_point`, `set_min`, `set_max`, and `intersects_ray(origin, dir, t_out, normal)` (slab test, `tmin` starts at 0.001 and `tmax` at 1000). `is_dirty()`/`set_dirty()` drive `SceneObject`'s cached world AABB.

## TriangleMeshCollider

`TriangleMeshCollider` stores world-space triangles in a BVH (leaf size 12, split along the widest centroid axis). Queries:

- `blocks_vertical_capsule(x, z, from_x, from_z, feet_y, head_y, radius)` — samples three sphere centres (feet + radius, middle, head − radius) and compares the closest distance at the target position against the current position, so an actor can move out of existing penetration;
- `raycast(origin, direction, max_distance, out_distance)` — Möller–Trumbore test over BVH candidates; direction is expected to be normalised;
- `find_ground_height(x, z, max_y, min_y, out_y)` — highest up-facing triangle (normal Y at least half the normal length) sampled at five points within 0.4 m;
- `find_landing_surface(x, z, radius, previous_feet_y, feet_y, minimum_up_normal, out_y)` — highest walkable surface crossed while descending.

## CollisionWorld

`CollisionWorld` owns a list of `TriangleMeshCollider`s. `add_static_mesh(triangles, world_transform)` builds one and returns its triangle count. The query methods (`blocks_vertical_capsule`, `raycast`, `find_ground_height`, `find_landing_surface`) run over all registered colliders and return the best result. `clear()` removes all colliders.

Registration is driven by whoever owns the geometry: RivalGrounds builds one collider from the city mesh and forwards scene collision hooks to it.

## PhysicsComponent

Each `SceneObject` owns a `PhysicsComponent` bound to its transform and the scene's fixed step. It stores linear and angular momentum, accumulated force/torque, mass and inverse mass, an inverse inertia tensor, gravity and enabled flags.

- `apply_force`, `apply_impulse_at_point`, `clear_forces`
- `integrate_velocity()` applies force/torque over the fixed step
- `integrate_position()` moves and rotates the transform by the momentum
- `set_mass`, `set_gravity`, `enable`, `disable`, momentum setters

A disabled component is ignored by the scene simulation, and a disabled or infinite-mass component counts as static.

## Scene simulation

`Scene::step_simulation()` (1/60 s) applies gravity at 14 m/s², damps linear momentum by 0.95 and angular momentum by 0.85 per step, integrates, then:

- resolves dynamic-vs-dynamic overlap with an XZ spatial hash (`GRID_CELL_SIZE` 2.2 m) and lifts the upper box by up to 1 m, damping its motion;
- settles gravity-enabled dynamic objects whose AABB bottom falls below the ground surface found by `find_ground_surface()`.

Games add scene colliders as AABBs through `Scene::collect_colliders_recursive` or as meshes through a `CollisionWorld`.
