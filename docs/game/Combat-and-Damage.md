# Combat and damage

Combat is symmetric hitscan: the player and enemies both trace a ray and apply damage instantly. There are no projectiles, explosions, or hitboxes beyond AABBs.

## Player shot resolution (left click)

1. Start at the player eye, direction from the camera.
2. Nearest environment hit among scene AABBs and the city collision mesh (max 200 m).
3. Enemy hitboxes tested with `BoundingBox::intersects_ray`. The nearest enemy closer than the environment hit takes 65 damage with impulse `dir * 0.4`.
4. A tracer is drawn for 0.08 s from just in front of the camera to the hit point (yellow for the player).
5. Physics objects on the ray receive an impulse and angular kick.

Enemy hitboxes are AABBs, so shots are resolved against the box, not the visible body parts.

## Grabbing and hiding

`handle_object_grab()` runs every frame. Right click picks the nearest physics
crate within 4 m of the view ray (it becomes non-collidable and loses physics),
or throws the held crate when one is already held, snapping it on top of or next
to the crate it is aimed at. While held, the crate is repositioned 2.2 m in front
of the camera.

`H` toggles `m_held_object_hidden`, and the held crate's `visible` flag follows
that state each frame (`visible = !m_held_object_hidden`). Because input is
polled, the toggle is edge-detected with `m_hide_key_was_pressed`, so holding `H`
does not flip the crate every frame. Grabbing resets the state to visible, and
throwing always sets `visible = true` so a crate never lands invisible.

## Enemy shot resolution

`Enemy::update()` reports a shot and the muzzle position. The game then:

1. computes the direction to the player and the distance;
2. raycasts the city mesh and scene AABBs toward the player;
3. discards damage if geometry blocks the shot before `distance - 0.15`;
4. otherwise applies 10 damage unless the player is in spawn grace or debug-invincible;
5. adds a red tracer from the muzzle to the first obstruction or near the player.

Damage sets a 0.28 s damage flash, a 6 s health-regen cooldown, and can end the match in defeat.

## Health and regeneration

- Player: 100 max, regenerates 4 HP/s after 6 s without damage. A 2.5 s spawn grace starts with the match, and touching down after the drop-in refreshes it to 3 s so the fall cannot be punished.
- Enemy: 300 max, regenerates 15 HP/s after 4 s without damage. Player shots do 65, so five hits kill an enemy before regeneration can outpace the damage.

## Feedback

- Hit marker (crossed lines) for 0.22 s when the player hits an enemy.
- Enemy damage flash mixes the body colour toward red.
- Damage vignette (red screen overlay) while the player's damage flash timer is active.
- Crosshair gap reacts to recoil.

## Tracers

`BulletTracer` is a plain struct (`start`, `end`, lifetime, colour) stored in a vector. Lifetimes are reduced by the fixed step and expired entries are erased. Tracers are data only; nothing is rendered from them in the current HUD, which draws no tracer geometry.
