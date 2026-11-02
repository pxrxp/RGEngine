# Rendering and post-processing

## Scene rendering

The game submits a tree of `SceneObject`s. Each object has a mesh, a material, and a render lambda that sets `u_view`, `u_proj`, `u_model` (through `Scene::set_*_matrix`) and calls `packet->mesh->draw()`. `Renderer` sorts by priority, program, and material.

Two priorities matter:

- normal objects (city, props, enemies, scroll, skybox) use `NORMAL_PRIORITY`;
- the weapon viewmodel uses `LOW_PRIORITY` and clears depth before drawing, so it always appears in front.

The skybox is drawn with a view matrix stripped of translation and depth function `LEQUAL`.

## Game shaders

`game/shaders/cube.vert` transforms position/normal and passes world position and UV. `cube.frag` is the game's only material shader:

- uniforms: `u_view`, `u_color`, `u_diffuse`, `u_use_texture`, `u_is_gun`, `u_preserve_color`;
- a fixed directional light plus a hemisphere ambient;
- a Blinn–Phong specular term (stronger for the gun);
- gamma 2.2 correction;
- writes `out_preserve_color` (attachment 1) from `u_preserve_color`.

Objects that should stay coloured in the grayscale world (enemies, weapon, the scroll, colour props) set `u_preserve_color = 1` around their draw call.

## Post-process

`game/shaders/grayscale.frag` is registered by `main.cpp` via `App::set_post_process()`. It samples `u_scene` (resolved colour) and `u_scene_aux` (resolved mask), computes luminance with Rec. 601 weights scaled to 0.25, and mixes back to full colour where the mask is 1. The engine only resolves the target and draws the fullscreen triangle; the effect itself is the game's.

## HUD rendering

The HUD is drawn with ImGui draw lists (`ImGui::GetForegroundDrawList()`), not with scene meshes. `App::render_debug_gui()` renders that draw data after the post-process pass, so HUD elements are never grayscaled.

## Debug drawing

The F1 menu's hitbox view projects AABB corners with the view-projection matrix and draws lines through the foreground draw list. It also draws a text count of boxes and how many were visible.
