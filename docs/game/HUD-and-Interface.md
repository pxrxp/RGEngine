# HUD and interface

All HUD drawing is in `RivalGrounds::render_game_hud()` using ImGui draw lists. Nothing in the HUD is a scene object.

## Intro screen

- Full-screen near-black overlay.
- `logo` texture scaled to at most 30% of the window width and 320 px (aspect ratio preserved) when it loaded successfully.
- Four-paragraph backstory between the logo and the controls.
- Control list: WASD move, Ctrl sprint, Shift sneak, Space jump, left click fire, right click grab/place crate.
- "Press Space or Click to Begin".

## During play

| Element | Details |
| --- | --- |
| Crosshair | Four lines with a gap that grows with recoil (`5 + recoil * 50` px) |
| Hit marker | Crossed lines for 0.22 s after hitting an enemy |
| Damage flash | Red full-screen overlay fading over 0.28 s |
| Health bar | 200 × 14 px bottom-left, green fill proportional to health |
| Minimap | 70 px radius circle bottom-right, 160 m range: player (green), scroll (gold), enemies (red) |
| Scroll distance | "Scroll distance N m" at top centre while playing |
| Enemy health bars | Over combat-state enemies within 35 m that are not occluded by colliders |

## Victory and defeat

Centred text with a one-pixel shadow: "You found the scroll" / "Press R to play again" and "You were taken down" / "Press R to retry".

## Debug menu

The F1 menu is a normal ImGui window (`RivalGrounds Debug (F1)`) with tabs for Player, Camera, World/Physics, and Enemies; see [Debug tools](Debug-Tools.md). It is only compiled in debug builds (`#ifndef NDEBUG`).

## No ammo or weapon UI

Because the game has a single weapon without ammunition, the HUD has no ammo counter, weapon name, or switching indicator.
