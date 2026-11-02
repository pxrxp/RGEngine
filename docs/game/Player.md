# Player

The player is a `SceneObject` at eye height. Physics is disabled on it (`get_physics_component().disable()`, `set_collidable(false)`); the game moves it directly and the camera follows it.

```cpp
m_player = std::make_shared<SceneObject>();
m_player->get_physics_component().disable();
m_player->set_collidable(false);
m_scene_ptr->add_child(m_player);
m_app_cache.get_camera().setup(m_player);
```

The camera applies mouse look by rotating the player object, and the controller derives forward/right from that rotation. The camera therefore has no position of its own: the player object is the eye.

## Constants

| Value | Constant |
| --- | --- |
| Eye height 1.7 m, radius 0.28 m | `PLAYER_EYE_HEIGHT`, `PLAYER_RADIUS` |
| Walk 7 m/s, sprint 11.5 m/s (Ctrl), sneak 3.8 m/s (Shift) | controller |
| Jump velocity 7.2 m/s, gravity 18 m/s² | controller |
| Body extent for collision 1.65 m | `head_y = feet_y + 1.65f` |
| Max health 100, regen 4 HP/s after a 6 s cooldown | `m_player_health`, `m_player_regen_cooldown` |
| Spawn grace 10 s after starting, refreshed to 10 s on landing | `m_spawn_grace_timer`, `m_awaiting_landing` |

## Movement

`update_character_controller(dt)` runs only while the cursor is disabled.

1. Build a wish direction from WASD/arrow keys, project it onto the horizontal plane using the player's rotation, normalise it.
2. Move each axis separately; each candidate position is tested against scene colliders (circle vs AABB with an allowance for already-overlapping colliders) and against `static_mesh_horizontal_blocked()` (city mesh capsule test). A blocked axis is simply not applied.
3. Jump on `Space` with a 0.25 s cooldown and only when grounded.
4. Integrate vertical velocity with 18 m/s² gravity, then look for a landing surface: the ground plane at y = 0, scene AABBs whose top edge is crossed, `CollisionWorld::find_landing_surface()`, and the walkable crate tops.
5. Clamp X/Z to the city bounds when known.

The player never uses `Scene`'s dynamic physics; the controller is its own kinematic solver.

## Health

Enemy shots reduce `m_player_health` by 6 (before spawn grace ends) and set a 6 s regen cooldown and a 0.28 s damage flash. After the cooldown, health regenerates at 4 HP/s up to 100. Reaching 0 sets `GameState::DEFEATED`.

## Crates

Right click picks the nearest physics object within 4 m of the camera ray, disables its physics and collidability, and moves it 2.2 m in front of the camera each frame. Right click again places it: on top of the hit object if the hit normal points up, beside it otherwise, or 2.2 m ahead at crate half-height if nothing is hit. `H` toggles the carried crate's visibility; the hidden state resets when the crate is grabbed or thrown.

## Spawn

`reset_match()` first places the player at `m_player_spawn`, then chooses a random city position at `max_y + 12` and drops the player in, avoiding the origin. `m_player_vy` is reset and the camera is pointed at `spawn + (0,0,-1)`.
