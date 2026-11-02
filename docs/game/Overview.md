# Overview

| | |
| --- | --- |
| Genre | Single-player first-person shooter |
| Goal | Find the gold scroll hidden in the city |
| Player | 100 HP, regenerating, one hitscan rifle, can grab and place crates |
| Enemies | Soldiers in patrol/combat states, 300 HP, fire hitscan shots |
| World | A city model scaled to 18×, invisible boundary walls, a ground plane, 128 physics crates |
| Win | Walk within 2.2 m of the scroll (and within 2 m vertically) |
| Lose | Player health reaches 0 |
| Restart | `R` from intro, victory, or defeat |

The game is a focused vertical slice: one map, one weapon, one enemy type, one objective. All of it lives in `RivalGrounds` (`game/include/rivalgrounds/RivalGrounds.h`) plus `Enemy`.

## What the game uses from the engine

- `App` for the window, frame timing, scene render target, and repositories.
- `Scene` as the game base class: fixed-step physics, collision hooks, render submission, uniform helpers.
- `SceneObject` trees for the city, props, player, weapon viewmodel, objective, and barrier walls.
- `CollisionWorld`/`TriangleMeshCollider` for the city mesh.
- `MaterialRepo`/`MeshRepo`/`TextureRepo` for city groups, props, the viewmodel, the scroll, and the logo.
- ImGui draw lists for the HUD and the debug menu.

## What the game adds

- A character controller with sprint/sneak, jump, ground snapping, and city bounds clamping.
- Hitscan combat with tracers, hit markers, damage flash, health regeneration, spawn grace, and victory/defeat.
- Enemy patrol/combat AI with line-of-sight, hearing, strafing, reinforcements, and death visuals.
- The grayscale-with-highlights post-process, driven by the game's shaders writing the auxiliary attachment.
- Crates that can be shot, pushed, picked up, carried, and stacked.
