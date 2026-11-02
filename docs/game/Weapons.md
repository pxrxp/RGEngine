# Weapons

The game has one weapon: a hitscan rifle with an M4 Colt viewmodel. There is no ammo, no reload, no weapon switching, and no grenade.

## Viewmodel

`setup_weapon_viewmodel()` loads `M4Colt.obj` with `from_OBJ<Vertex_PosTexNorm, TriangleIndices>` and the `m4_colt` texture through the material repository (`cube_program`, `u_is_gun = 1`). The render lambda builds a view-space transform:

```cpp
translate(0.24, -0.24, -0.36 + recoil) *
rotate(98°, Y) * rotate(-3°, X) * rotate(6°, Z) *
rotate(recoil * 35°, Z) * scale(0.065)
```

and converts it to world space with `inverse(view)`, then sets `u_view`, `u_proj`, `u_model`. Before drawing it clears the depth buffer so the gun is never clipped by the world. The packet uses `LOW_PRIORITY`, so it is drawn after every world object.

## Firing

`handle_shooting()` reacts to the left mouse button going down (not while held):

- sets `m_gun_recoil = 0.11` (decays exponentially at 18/s);
- builds a collider list from scene AABBs;
- raycasts the camera ray against the city mesh, then each AABB, keeping the nearest hit (`env_t`, default 200 m);
- raycasts enemy hitboxes; the nearest enemy closer than `env_t` takes 65 damage and gets an impulse of `dir * 0.4`; a hit marker is shown for 0.22 s;
- adds a short-lived (0.08 s) tracer from `origin + dir * 0.4` to the hit point, or to the environment hit;
- applies an 8 N impulse plus upward lift to any physics object along the ray within `env_t`, with a fixed angular kick.

There is no spread, falloff, or ammo. Damage is applied immediately.

## Recoil in the HUD

The crosshair gap widens with `m_gun_recoil` (`5 + recoil * 50` pixels), so each shot produces a visible kick without changing aim direction.
