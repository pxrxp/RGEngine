# Enemy AI

Enemies have three states: `PATROL`, `COMBAT`, `DEAD`. `Enemy::update()` advances the state machine each fixed step.

## Per-step outline

1. If dead, return immediately.
2. Decay damage flash, shoot cooldown, and regen cooldown; regenerate health when off cooldown.
3. Compute the muzzle position (`pos + (0.30, 1.25, -0.45)`).
4. Perceive: horizontal distance to the player (hearing), and `has_line_of_sight()`.
5. If seen or heard, switch to `COMBAT` and set a 7 s alert timer.
6. In combat: face the player, choose a target position (chase beyond 18 m, back off inside 7 m, strafe at 3.5 m otherwise), and fire when the aim is not too vertical, the cooldown is ready, and the player is within 36 m.
7. In patrol, advance to the next waypoint within 2.5 m.
8. Move toward the target position at 3.4 m/s in combat or 2.0 m/s in patrol, testing X and Z separately, then snap to the ground.

## Line of sight

`has_line_of_sight()` returns false when:

- the player is farther than 64 m or closer than 0.1 m;
- the vertical component of the aim direction exceeds 0.65;
- in patrol, the player is outside the forward cone (`dot < 0.35`);
- any collider AABB intersects the ray between 0.4 m and `distance - 0.4`;
- a city mesh raycast hits inside that range.

Combat ignores the view cone, so a spotted player stays visible as long as there is no cover.

## Movement and collision

Each step tests `pos + delta.x` and then `pos.z + delta.z` against collider AABBs (0.5 m radius, respecting the enemy's height range) and the city mesh capsule test with the enemy's radius and height. A blocked patrol step advances to the next waypoint. After moving, `find_actor_ground_surface()` snaps the enemy to the surface below it.

## Firing

Firing sets `out_fired_shot` and `out_damage = 16`. The game then checks the shot line against the world; if geometry is closer than the player, damage is discarded and the tracer is shortened to the obstruction. Firing also creates a short red tracer in the world.

## Alert decay

The alert timer decreases in combat. When it reaches zero and the player is neither visible nor within hearing range, the enemy returns to patrol and targets its first waypoint.
