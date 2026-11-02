# Collision system

Collision has three independent parts:

1. **City mesh** — a `CollisionWorld` with one `TriangleMeshCollider` built from the city's triangles and world transform. Used for capsule blocking, raycasts, ground height, and landing surfaces.
2. **Scene AABBs** — every collidable scene object with a render packet contributes its world AABB (`collect_colliders_recursive`). This covers the crates and the invisible boundary walls. Enemy hitboxes are appended when a query asks for nearby enemies.
3. **Ground plane** — the controller additionally treats y = 0 as ground when the player crosses it downward.

## How the game exposes the engine hooks

`RivalGrounds` overrides:

```cpp
bool blocks_vertical_capsule(...) const override;        // -> m_collision_world.blocks_vertical_capsule
bool static_mesh_ray_hit(...) const override;            // -> m_collision_world.raycast
bool find_ground_surface(...) const override;            // -> m_collision_world.find_ground_height
bool find_ground_surface_for_actor(...) const override;  // same
bool find_landing_surface(...) const override;           // -> find_landing_surface(..., 0.45f)
```

Scene physics, the player controller, enemy movement, and combat raycasts all go through these hooks.

## Player collision

The controller tests candidate positions as circles (radius 0.28 m) against AABBs in the player's height band, plus the mesh capsule test. Each axis is applied only if it is not blocked; if the player already overlaps an AABB, movement that reduces penetration is allowed so the player cannot get stuck.

Landing checks the highest crossed top edge among the scene AABBs and mesh landing surfaces within the player's radius.

## Enemy collision

Enemies use a 0.5 m test radius for AABB blocking and their 0.48 m radius for the mesh capsule test, with a height band from their feet to feet + 2.2 m. They also snap to the ground surface after each step. Invisibility barriers and crates block them the same way they block the player.

## Dynamic bodies

Crates use `PhysicsComponent` (mass, gravity, momentum) rather than triangle collision. `Scene::step_simulation()` resolves crate-versus-crate overlap by lifting the upper crate, and settles crates that fall below the ground surface. Crates do not collide with the city mesh horizontally; only their ground settling query uses it.

## Limitations

- No capsule-vs-capsule or triangle-vs-box resolution.
- No slopes beyond ground/landing queries with an upward-normal threshold.
- Enemy-vs-enemy collision is not handled; enemies can overlap each other.
- AABB colliders ignore rotation (the city and crates use axis-aligned or uniformly scaled boxes).
