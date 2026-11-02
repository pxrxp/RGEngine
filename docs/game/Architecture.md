# Game architecture

## RivalGrounds

`RivalGrounds` derives from `Scene` and is declared entirely in `game/include/rivalgrounds/RivalGrounds.h`. There is no separate `.cpp` for its gameplay code; only the debug menu (debug builds) is compiled from `RivalGroundsDebug.cpp`. This is intentional for a single-file vertical slice, but it means the header is large.

State it owns:

| Group | Members |
| --- | --- |
| Session | `GameState m_game_state`, `m_logo_texture`, `m_scroll_pos`, `m_scroll_objective` |
| Player | `m_player`, health/regen, vertical velocity, grounded/jump state, spawn grace |
| Weapon | `m_weapon_viewmodel`, `m_gun_recoil`, mouse latch, tracers, hit marker, damage flash |
| Interaction | `m_held_object`, `m_held_object_hidden`, hide-key latch |
| World | `m_collision_world`, `m_city_world_min/max`, `m_city_bounds_valid`, `m_enemies`, `m_reinforcement_timer` |
| Debug | `m_debug_menu_open`, `m_debug_noclip`, `m_debug_invincible`, `m_debug_show_hitboxes` |

## Enemy

`Enemy` (`game/include/rivalgrounds/Enemy.h`) owns its scene subtree: a root object with eight box parts (legs, torso, head, visor, three gun pieces) sharing a `shared_ptr<EnemyVisualState>` that render lambdas read weakly for damage/death colouring. It keeps its own health, cooldown, alert timer, waypoints, spawn point, and target position.

`Enemy::update()` is called by the game each fixed step with the player position, camera forward, collider list, and the fixed delta time. It reports damage, whether it fired, and its muzzle position back through references.

## main.cpp

`game/src/main.cpp` owns the frame:

```cpp
App app(AppConfig{ .window_title = "RivalGrounds" });
app.init();
app.get_window().set_cursor_mode(GLFW_CURSOR_DISABLED);
app.set_post_process(ShaderProgramDescriptor{
  .program_name = "grayscale_postprocess",
  .shaders = { "postprocess.vert", "grayscale.frag" } });

auto game_scene = std::make_shared<RivalGrounds>(app);
game_scene->init();

while (app.is_running()) {
  app.update();
  app.init_debug_gui();
  app.begin_scene_render();
  app.get_window().clear(RGBA_BLACK, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  game_scene->update_physics();
  game_scene->submit_to_renderer();
  app.get_renderer().render();
  app.end_scene_render();
  app.render_debug_gui();
  app.get_window().swap_buffers();
}
```

The scene is destroyed before `app.cleanup()`, so meshes and materials are released before the repositories are cleared.

## Split from the engine

Nothing in `engine/` refers to `RivalGrounds`, the M4 Colt, the city, the grayscale look, or the HUD. Window title, cursor policy, post-process shader, and shaders defining the game's look all live in `game/`.
