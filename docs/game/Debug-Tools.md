# Debug tools

Debug features are compiled only when `NDEBUG` is not defined. `build.sh`/`build.bat` take `BUILD_TYPE` (use `Debug`), and CMake defaults ASan/UBSan on for debug work.

## F1 menu

`F1` toggles the `RivalGrounds Debug` window and releases/captures the cursor. Tabs:

| Tab | Contents |
| --- | --- |
| Player | Noclip (WASD + E/Q), invincible, show collision hitboxes, position/health/grounded/vertical velocity, game state integer, teleport by X/Y/Z, teleport to spawn, spawn an enemy nearby |
| Camera | Forward vector, FOV, move speed, sensitivity, near/far planes, sliders for FOV (35–110°), speed (0.25–30), sensitivity (0.05–2) |
| World / Physics | Static collision mesh count, physics object count, fixed step |
| Enemies | Total/patrol/combat/dead counts and a table of index, state, position, health, damage flash |

The menu is drawn from `render_game_hud()` *before* the rest of the HUD, and only when the window is open. Closing it restores `GLFW_CURSOR_DISABLED`.

## Collision hitboxes

With "Show collision hitboxes" enabled, every collidable scene AABB and every living enemy hitbox is projected to screen space and drawn as 12 lines on the foreground draw list. A label reports the number of boxes and how many projected on screen.

## Noclip

Noclip replaces the character controller: it moves the player along the camera direction without collision or gravity and ignores jump/landing checks. It also skips building the collider list.

## Invincibility

Invincibility skips applying enemy shot damage while still showing tracers.

## Other runtime output

- `Rival Grounds: city mesh loaded at scale ... (N collision triangles); boundary walls added.` — printed when the city is built.
- `Scene antialiasing: Nx MSAA` or `single-sample fallback` — printed on first scene framebuffer creation.
- GL debug messages from `Util::init_gl_debug()`.
- Repository load failures print shader/link logs and missing texture or mesh names.
