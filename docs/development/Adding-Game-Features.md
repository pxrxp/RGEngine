# Adding game features

1. **Decide which object owns it.** Player state and rules belong in `RivalGrounds`; enemy behaviour in `Enemy`; anything reusable goes to the engine.
2. **Add state** as private members in `RivalGrounds.h` with a default, and reset it in `reset_match()` if it is per-match.
3. **Update it** inside `update_physics()` while playing. Use `Scene::FIXED_STEP`, not `get_delta_time()`, so gameplay stays frame-rate independent.
4. **Add content** by creating `SceneObject`s in `reset_match()` (or a helper called from it), loading meshes and materials through `m_app_cache.mesh_repo` / `material_repo`.
5. **Draw it** with `set_render_packet(..., capture_weak(object, lambda))`. Use `Scene::set_view_matrix`, `set_projection_matrix`, `set_model_matrix`. Set `u_preserve_color` if it must keep its colour through the grayscale pass.
6. **Add UI** in `render_game_hud()` using ImGui draw lists, or in `RivalGroundsDebug.cpp` for developer-only controls.
7. **Reset it** in `reset_match()` so restarts are deterministic.

## Example: a second weapon

The code has no weapon class. To add one you would:

- add members for the current weapon and its cooldown to `RivalGrounds`;
- branch in `handle_shooting()` on the selected weapon, sharing the ray/tracer code;
- add a key binding in `update_physics()`;
- add the viewmodel as a second `SceneObject` (or swap meshes) and hide the inactive one (`visible = false`);
- surface the choice in the HUD.

## Rules to keep

- Do not add game logic to `engine/`.
- Do not put game shaders in `engine/shaders`.
- Keep at least one code path that resets all per-match state.
