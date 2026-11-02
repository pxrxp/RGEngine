# Game loop

The loop is the one shown in [Architecture](Architecture.md): `App` handles events, timing, the offscreen scene target, and presentation; `RivalGrounds` handles gameplay and UI.

## Per frame

1. `App::update()` — poll GLFW events, apply mouse look if the cursor is locked, clear the mouse delta, update delta time.
2. `App::init_debug_gui()` — start an ImGui frame.
3. `App::begin_scene_render()` — bind the MSAA scene target and clear the auxiliary channel.
4. `Window::clear()` — clear colour and depth.
5. `RivalGrounds::update_physics()` — gameplay update (see below).
6. `RivalGrounds::submit_to_renderer()` — submit the scene, then draw the HUD/debug menu draw lists.
7. `App::get_renderer().render()` — sort and draw the render packets.
8. `App::end_scene_render()` — resolve, run the grayscale post-process, restore state.
9. `App::render_debug_gui()` — ImGui draw data on top of the post-processed frame.
10. `Window::swap_buffers()`.

## update_physics()

`RivalGrounds::update_physics()` first handles global input:

- `ESC` releases the cursor, `TAB` captures it;
- outside the playing state, `R` restarts the match;
- in the intro state, `Space`, `Enter`, or left click starts play.

Then, while playing:

1. `Scene::update_physics()` — run zero or more fixed 1/60 s simulation steps.
2. Decay timers: recoil, damage flash, hit marker, spawn grace, health regen.
3. Move the carried crate in front of the camera.
4. Age and drop bullet tracers.
5. Debug: `F1` toggles the debug menu and the cursor.
6. `update_character_controller(dt)` — movement, jump, gravity, landing, bounds.
7. `handle_shooting()` — resolve a hitscan shot on click.
8. `handle_object_grab()` — pick up / place a crate on right click.
9. Collect colliders (scene AABBs plus living enemy hitboxes).
10. Cull enemies farther than 170 m from the player, replacing living ones at 60–90 m.
11. Reinforcement timer — spawn an enemy every 14 s at 45–75 m while fewer than 14 are alive.
12. Update every living enemy; resolve their shots against the world; apply damage.
13. Check the scroll objective for victory.

## Fixed vs frame time

Gameplay inside the playing state uses `Scene::FIXED_STEP` (1/60 s) for the controller, enemy updates, and timers, not the frame delta. Enemy fire rates, movement speeds, and regen rates are therefore stable regardless of frame rate. If the accumulator falls behind, `Scene::update_physics()` runs several steps in one frame (capped at 0.2 s of backlog).

## HUD

`submit_to_renderer()` calls `Scene::submit_to_renderer()` and then `render_game_hud()`, which draws the F1 debug menu (debug builds), the intro/victory/defeat overlays, crosshair, hit marker, damage flash, health bar, minimap, scroll distance, and enemy health bars — all through ImGui draw lists.
