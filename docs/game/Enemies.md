# Enemies

`Enemy` (`game/include/rivalgrounds/Enemy.h`) is a soldier made of eight box parts under one root object.

## Stats

| Value | |
| --- | --- |
| Health 300, regenerates 15 HP/s after a 4 s delay | `m_health`, `m_max_health`, `m_regen_cooldown` |
| Collision radius 0.48 m, height 2.2 m | `ENEMY_COLLISION_RADIUS`, `ENEMY_COLLISION_HEIGHT` |
| Vision and hearing range 64 m | `MAX_VISION_RANGE`, `HEARING_DISTANCE` |
| Vision cone cosine 0.35 while patrolling | `FOV_COSINE` |
| Shot damage 16, cooldown 0.9 s, range 36 m | `Enemy::update` |
| Knockback scale 0.5 on damage | `ENEMY_KNOCKBACK_SCALE` |
| Hitbox 1.1 × 2.3 × 1.1 m around the root | `get_hitbox()` |

## Body and visuals

Parts use `sharp_box_mesh` and a shared `cube_program` material per part name, each with its own colour uniform. Render lambdas capture a `weak_ptr<EnemyVisualState>`; the state carries the enemy's `EnemyState` and damage flash, so the body turns dark red when dead and flashes red/white when hit without the render callback keeping the enemy alive.

## Spawning

- `spawn_tactical_enemies()` places 14 enemies at random city positions at least 45 m from the player, each with a two-waypoint patrol route offset by up to ±24 m.
- The reinforcement timer spawns one more enemy every 14 s at 55–115 m from the player (random angle) while fewer than 14 enemies are alive.
- The F1 debug menu can spawn an extra enemy 8 m away.

Enemies spawn at y = 0.85 and settle onto the ground through the movement code.

## Death

Taking damage to 0 health sets `EnemyState::DEAD`, drops the body 0.5 m, rotates it 90° around X, colours it dark red, and stops all further updates. Dead enemies are skipped by combat, HUD indicators, and minimap dots. There is no corpse cleanup and no respawn of a specific enemy; reinforcements keep coming.
