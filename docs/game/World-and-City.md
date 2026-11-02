# World and city

`reset_match()` rebuilds the whole world on every match.

## City model

`build_massive_city()` loads `city.obj` (16 MB, ~261k collision triangles) twice:

- `from_OBJ<Vertex_PosTexNorm, TriangleIndices>` for geometry and collision;
- `from_OBJ_material_groups<...>` for the OBJ's material groups.

Both are cached in function-local statics, so restarting a match reuses the parsed data.

The model is scaled by 18× (`CITY_MODEL_SCALE`) and centred so that the model's road height (`CITY_ROAD_LOCAL_Y = 0.425118`) maps to y = 0.

- If material groups exist, each group becomes a `SceneObject` with its own mesh, `cube_program` material, diffuse colour, and texture when the MTL declares `map_Kd`. Texture names are the MTL texture path without `resources/` and without its extension, and use `GL_SRGB8_ALPHA8`.
- If the OBJ has no groups, the whole mesh is drawn once with the `concrete` texture.

The mesh's collision triangles are registered with `m_collision_world.add_static_mesh(triangles, city_world)`.

## Bounds and boundary

`m_city_world_min/max` come from the transformed model AABB. Four invisible, collidable `SceneObject`s (no render packet, physics disabled) form walls around the city, at least 24 m tall and 6 m thick. The player's controller also clamps X/Z inside the bounds.

## Ground plane

`setup_plane()` adds a textured quad (`city_under_plane_mesh_flush`) at y = −0.02, scaled to the city's X/Z extent, with 120× repeated `concrete` UVs. It is visual only; the controller treats y = 0 as the base ground.

## Skybox

A cube-mapped skybox (`skybox_*` PNG faces) is added with `setup_skybox()` and drawn with depth `LEQUAL` at infinite distance.

## Props

`reset_match()` spawns 128 `Box.obj` physics crates at random positions at least 16 m from the player, with a 2.5 kg mass, gravity enabled, and collidable. They are the game's movable cover and can be shot, pushed, grabbed, and stacked.

## Objective

The scroll is a gold `sharp_box_mesh` drawn with `u_preserve_color = 1` (so the grayscale pass keeps it coloured), placed at a random position at least 25 m from the player with an open ceiling (a ray straight up must hit no mesh or collider). Touching it wins the match.
