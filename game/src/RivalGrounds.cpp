#include <rivalgrounds/RivalGrounds.h>

#include <engine/Material.h>
#include <engine/ShaderProgram.h>

#include <engine/repos/MeshRepo.h>
#include <engine/util/obj_model.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <random>

void
RivalGrounds::init()
{
  Scene::init();
#ifndef NDEBUG
  debug_camera();
#endif

  m_player = std::make_shared<SceneObject>();
  m_player->get_physics_component().disable();
  m_player->set_collidable(false);
  m_scene_ptr->add_child(m_player);
  m_app_cache.get_camera().setup(m_player);

  setup_weapon_viewmodel();

  m_logo_texture = m_app_cache.material_repo.tex_repo.load_texture(
    TextureDescriptor{ .texture_name = "logo",
                       .wrap_s = GL_CLAMP_TO_EDGE,
                       .wrap_t = GL_CLAMP_TO_EDGE,
                       .generate_mipmaps = false });

  reset_match();
  m_game_state = GameState::INTRO;
}

void
RivalGrounds::setup_weapon_viewmodel()
{
  m_weapon_viewmodel = std::make_shared<SceneObject>();
  m_weapon_viewmodel->get_physics_component().disable();
  m_weapon_viewmodel->set_collidable(false);

  auto mesh = m_app_cache.mesh_repo.load_mesh(
    from_OBJ<Vertex_PosTexNorm, TriangleIndices>("M4Colt.obj", GL_TRIANGLES));

  auto mat = m_app_cache.material_repo.load_material(MaterialDescriptor{
    .material_name = "m4_colt_material",
    .shader_program_desc = { .program_name = "cube_program",
                             .shaders = { "cube.vert", "cube.frag" } },
    .texture_desc_list = { { .texture_name = "m4_colt" } },
    .uniforms = { { "u_diffuse", TextureName("m4_colt") },
                  { "u_use_texture", 1 },
                  { "u_is_gun", 1 } } });

  m_weapon_viewmodel->set_render_packet(
    mesh,
    mat,
    capture_weak(
      m_weapon_viewmodel,
      [this]([[maybe_unused]] SceneObjectStrongPtr self,
             RenderPacketStrongPtr packet) {
        glm::mat4 V = m_app_cache.get_camera().get_view_matrix();
        glm::mat4 P = m_app_cache.get_camera().get_projection_matrix();

        glm::mat4 gun_in_view =
          glm::translate(glm::mat4(1.0f),
                         glm::vec3(0.24f, -0.24f, -0.36f + m_gun_recoil)) *
          glm::rotate(
            glm::mat4(1.0f), glm::radians(98.0f), glm::vec3(0, 1, 0)) *
          glm::rotate(
            glm::mat4(1.0f), glm::radians(-3.0f), glm::vec3(1, 0, 0)) *
          glm::rotate(glm::mat4(1.0f), glm::radians(6.0f), glm::vec3(0, 0, 1)) *
          glm::rotate(glm::mat4(1.0f),
                      glm::radians(m_gun_recoil * 35.0f),
                      glm::vec3(0, 0, 1)) *
          glm::scale(glm::mat4(1.0f), glm::vec3(0.065f));

        glm::mat4 world_gun = glm::inverse(V) * gun_in_view;

        packet->material->get_shader_program()->set_uniform("u_view", V);
        packet->material->get_shader_program()->set_uniform("u_proj", P);
        packet->material->get_shader_program()->set_uniform("u_model",
                                                            world_gun);

        glDisable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        packet->material->get_shader_program()->set_uniform("u_preserve_color",
                                                            1);
        packet->mesh->draw();
        packet->material->get_shader_program()->set_uniform("u_preserve_color",
                                                            0);
      }));

  if (auto pkt = m_weapon_viewmodel->get_render_packet()) {
    pkt.value()->priority = LOW_PRIORITY;
  }
}

void
RivalGrounds::submit_to_renderer()
{
  Scene::submit_to_renderer();
  render_game_hud();
}

void
RivalGrounds::reset_match()
{
  m_game_state = GameState::PLAYING;
  m_player_health = 100.0f;
  m_spawn_grace_timer = 2.0f;
  m_held_object = nullptr;
  m_city_bounds_valid = false;
  m_enemies.clear();
  m_tracers.clear();
  m_collision_world.clear();

  m_player->get_local_transform().set_position(m_player_spawn);
  m_player_vy = 0.0f;
  m_player_grounded = true;
  m_space_was_pressed = false;
  m_jump_cooldown = 0.0f;
  m_app_cache.get_camera().look_at(m_player_spawn +
                                   glm::vec3(0.0f, 0.0f, -1.0f));

  m_scene_ptr = std::make_shared<SceneObject>();
  m_scene_ptr->add_child(m_player);

  if (m_weapon_viewmodel)
    m_scene_ptr->add_child(m_weapon_viewmodel);

  auto skybox = std::make_shared<SceneObject>();
  skybox->set_collidable(false);
  setup_skybox(skybox, "skybox");
  m_scene_ptr->add_child(skybox);

  std::random_device rd;
  std::mt19937 rng(rd());

  build_massive_city(rng);
  if (m_city_bounds_valid)
    setup_plane();

  const float drop_height = m_city_world_max.y + 12.0f;
  std::uniform_real_distribution<float> x_dist(m_city_world_min.x + 12.0f,
                                               m_city_world_max.x - 12.0f);
  std::uniform_real_distribution<float> y_dist(m_city_world_min.y + 12.0f,
                                               m_city_world_max.y - 48.0f);
  std::uniform_real_distribution<float> z_dist(m_city_world_min.z + 12.0f,
                                               m_city_world_max.z - 12.0f);

  glm::vec3 player_spawn(0.0f, drop_height, 0.0f);
  for (int attempt = 0; attempt < 100; ++attempt) {
    player_spawn.x = x_dist(rng);
    player_spawn.z = z_dist(rng);
    if (glm::distance(glm::vec2(player_spawn.x, player_spawn.z),
                      glm::vec2(0.0f)) > 18.0f)
      break;
  }
  m_player->get_local_transform().set_position(player_spawn);
  m_player_vy = 0.0f;
  m_player_grounded = false;
  m_space_was_pressed = false;
  m_app_cache.get_camera().look_at(player_spawn + glm::vec3(0.0f, 0.0f, -1.0f));

  for (int i = 0; i < 128; ++i) {
    const float rx = x_dist(rng);
    const float rz = z_dist(rng);
    if (glm::distance(glm::vec2(rx, rz),
                      glm::vec2(player_spawn.x, player_spawn.z)) < 16.0f)
      continue;
    spawn_physics_obj_prop("Box.obj",
                           glm::vec3(rx, drop_height / 4, rz),
                           glm::vec3(0.044f),
                           glm::vec3(0.220f, 0.110f, 0.063f));
  }

  m_scroll_pos = glm::vec3(x_dist(rng), y_dist(rng), z_dist(rng));
  for (int attempt = 0; attempt < 50; ++attempt) {
    const bool far_from_player =
      glm::distance(glm::vec2(m_scroll_pos.x, m_scroll_pos.z),
                    glm::vec2(player_spawn.x, player_spawn.z)) > 25.0f;

    if (far_from_player && has_open_ceiling(m_scroll_pos))
      break;

    m_scroll_pos.x = x_dist(rng);
    m_scroll_pos.z = z_dist(rng);
  }

  m_scroll_objective = std::make_shared<SceneObject>();
  m_scroll_objective->get_local_transform()
    .set_position(m_scroll_pos)
    .set_scale(glm::vec3(1.1f, 0.5f, 0.7f));
  m_scroll_objective->get_physics_component().disable();
  m_scroll_objective->set_collidable(false);
  m_scroll_objective->set_render_packet(
    m_app_cache.mesh_repo.load_mesh(get_sharp_box_descriptor()),
    m_app_cache.material_repo.load_material(MaterialDescriptor{
      .material_name = "scroll_gold_mat",
      .shader_program_desc = { .program_name = "cube_program",
                               .shaders = { "cube.vert", "cube.frag" } },
      .texture_desc_list = {},
      .uniforms = { { "u_color", glm::vec3(1.0f, 0.65f, 0.05f) },
                    { "u_use_texture", 0 },
                    { "u_is_gun", 0 } } }),
    capture_weak(
      m_scroll_objective,
      [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
        set_view_matrix(packet);
        set_projection_matrix(packet);
        set_model_matrix(packet, self->get_world_transformation_mat());

        auto shader_program = packet->material->get_shader_program();
        shader_program->set_uniform("u_preserve_color", 1);

        packet->mesh->draw();

        shader_program->set_uniform("u_preserve_color", 0);
      }));
  m_scene_ptr->add_child(m_scroll_objective);

  spawn_tactical_enemies(rng);
}

void
RivalGrounds::build_massive_city(std::mt19937& rng)
{
  (void)rng;
  constexpr float CITY_MODEL_SCALE = 18.0f;
  constexpr float BOUNDARY_WALL_THICKNESS = 6.0f;
  constexpr float BOUNDARY_WALL_MIN_HEIGHT = 24.0f;

  static const auto cached_city_desc =
    from_OBJ<Vertex_PosTexNorm, TriangleIndices>("city.obj", GL_TRIANGLES);
  auto desc = cached_city_desc;
  if (desc.vertices.empty() || desc.indices.empty()) {
    std::fprintf(stderr,
                 "Rival Grounds: city.obj has no triangle geometry; city not "
                 "spawned.\n");
    return;
  }

  glm::vec3 local_min(std::numeric_limits<float>::max());
  glm::vec3 local_max(std::numeric_limits<float>::lowest());
  for (const auto& vertex : desc.vertices) {
    local_min = glm::min(local_min, vertex.position);
    local_max = glm::max(local_max, vertex.position);
  }

  constexpr float CITY_ROAD_LOCAL_Y = 0.425118f;
  const glm::vec3 local_center((local_min.x + local_max.x) * 0.5f,
                               CITY_ROAD_LOCAL_Y,
                               (local_min.z + local_max.z) * 0.5f);
  const glm::vec3 world_offset = -local_center * CITY_MODEL_SCALE;
  const glm::mat4 city_world =
    glm::translate(glm::mat4(1.0f), world_offset) *
    glm::scale(glm::mat4(1.0f), glm::vec3(CITY_MODEL_SCALE));

  auto collision_mesh = m_app_cache.mesh_repo.load_mesh(desc);
  static const auto cached_city_groups =
    from_OBJ_material_groups<Vertex_PosTexNorm, TriangleIndices>("city.obj",
                                                                 GL_TRIANGLES);
  auto groups = cached_city_groups;
  if (groups.empty()) {
    auto city = std::make_shared<SceneObject>();
    city->get_physics_component().disable();
    city->set_collidable(false);
    city->get_local_transform()
      .set_position(world_offset)
      .set_scale(glm::vec3(CITY_MODEL_SCALE));
    city->set_render_packet(
      collision_mesh,
      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "downloaded_city_fallback",
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = { { .texture_name = "concrete" } },
        .uniforms = { { "u_diffuse", TextureName("concrete") },
                      { "u_color", glm::vec3(1.0f) },
                      { "u_use_texture", 1 },
                      { "u_is_gun", 0 },
                      { "u_use_uv", 1 } } }),
      capture_weak(
        city, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        }));
    m_scene_ptr->add_child(city);
  } else {
    for (auto& group : groups) {
      auto object = std::make_shared<SceneObject>();
      object->get_physics_component().disable();
      object->set_collidable(false);
      object->get_local_transform()
        .set_position(world_offset)
        .set_scale(glm::vec3(CITY_MODEL_SCALE));
      auto mesh = m_app_cache.mesh_repo.load_mesh(std::move(group.mesh));

      bool has_texture = !group.diffuse_texture.empty();
      std::string texture_key = "";

      if (has_texture) {
        std::filesystem::path rel_path =
          group.diffuse_texture.lexically_normal();

        std::string p_str = rel_path.generic_string();
        if (p_str.rfind("resources/", 0) == 0) {
          p_str = p_str.substr(10);
        }
        std::filesystem::path clean_path(p_str);
        clean_path.replace_extension();
        texture_key = clean_path.generic_string();
      }

      auto draw = capture_weak(
        object,
        [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        });

      MaterialDescriptor md{
        .material_name = "city_mat_" + group.material_name,
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list =
          has_texture
            ? std::vector<TextureDescriptor>{ { .texture_name = texture_key,
                                                .internal_format =
                                                  GL_SRGB8_ALPHA8,
                                                .wrap_s = GL_REPEAT,
                                                .wrap_t = GL_REPEAT,
                                                .generate_mipmaps = true } }
            : std::vector<TextureDescriptor>{},
        .uniforms =
          has_texture
            ? std::unordered_map<std::string,
                                 UniformValue>{ { "u_diffuse",
                                                  TextureName(texture_key) },
                                                { "u_color", group.diffuse },
                                                { "u_use_texture", 1 },
                                                { "u_is_gun", 0 } }
            : std::unordered_map<std::string, UniformValue>{ { "u_color",
                                                               group.diffuse },
                                                             { "u_use_texture",
                                                               0 },
                                                             { "u_is_gun", 0 } }
      };

      object->set_render_packet(
        mesh, m_app_cache.material_repo.load_material(md), std::move(draw));
      m_scene_ptr->add_child(object);
    }
  }

  const glm::vec3 world_min = (local_min - local_center) * CITY_MODEL_SCALE;
  const glm::vec3 world_max = (local_max - local_center) * CITY_MODEL_SCALE;
  m_city_world_min = world_min;
  m_city_world_max = world_max;
  m_city_bounds_valid = true;

  const float wall_height =
    std::max(BOUNDARY_WALL_MIN_HEIGHT, world_max.y - world_min.y);
  const float wall_mid_y = wall_height * 0.5f;
  const float span_x =
    std::max(BOUNDARY_WALL_THICKNESS, world_max.x - world_min.x);
  const float span_z =
    std::max(BOUNDARY_WALL_THICKNESS, world_max.z - world_min.z);
  const float half_x = span_x * 0.5f, half_z = span_z * 0.5f;
  const float cx = (world_min.x + world_max.x) * 0.5f,
              cz = (world_min.z + world_max.z) * 0.5f;
  const float ox = half_x + BOUNDARY_WALL_THICKNESS * 0.5f;
  const float oz = half_z + BOUNDARY_WALL_THICKNESS * 0.5f;

  auto add_invisible_barrier = [this](const glm::vec3& center,
                                      const glm::vec3& size) {
    auto barrier = std::make_shared<SceneObject>();
    barrier->get_local_transform().set_position(center).set_scale(size);
    barrier->get_physics_component().disable();
    barrier->set_collidable(true);
    m_scene_ptr->add_child(barrier);
  };

  add_invisible_barrier({ cx - ox, wall_mid_y, cz },
                        { BOUNDARY_WALL_THICKNESS,
                          wall_height,
                          span_z + 2.0f * BOUNDARY_WALL_THICKNESS });
  add_invisible_barrier({ cx + ox, wall_mid_y, cz },
                        { BOUNDARY_WALL_THICKNESS,
                          wall_height,
                          span_z + 2.0f * BOUNDARY_WALL_THICKNESS });
  add_invisible_barrier({ cx, wall_mid_y, cz - oz },
                        { span_x, wall_height, BOUNDARY_WALL_THICKNESS });
  add_invisible_barrier({ cx, wall_mid_y, cz + oz },
                        { span_x, wall_height, BOUNDARY_WALL_THICKNESS });

  const auto count = m_collision_world.add_static_mesh(
    collision_mesh->get_collision_triangles(), city_world);
  std::fprintf(stderr,
               "Rival Grounds: city mesh loaded at scale %.2f (%zu collision "
               "triangles); boundary walls added.\n",
               CITY_MODEL_SCALE,
               count);
}

void
RivalGrounds::spawn_tactical_enemies(std::mt19937& rng)
{
  if (!m_city_bounds_valid)
    return;

  // Scattering the squad over the whole city left most of them inside walls or
  // far out of reach, so place them in rings around the player and only on
  // ground the collision mesh actually agrees with.
  struct SpawnRing
  {
    int count;
    float min_distance;
    float max_distance;
  };
  const SpawnRing rings[] = {
    { 4, 20.0f, 30.0f },
    { 5, 32.0f, 50.0f },
    { 5, 52.0f, 74.0f },
  };

  constexpr float CITY_INSET = 12.0f;
  const glm::vec3 player_pos = m_player->get_local_transform().get_position();

  auto is_walkable = [this](const glm::vec3& p) {
    float ground = 0.0f;
    if (!m_collision_world.find_ground_height(
          p.x, p.z, p.y + 6.0f, p.y - 2.0f, ground))
      return false;
    return !m_collision_world.blocks_vertical_capsule(
      p.x, p.z, p.x, p.z, ground + 0.15f, ground + 1.75f, 0.4f);
  };

  std::uniform_real_distribution<float> angle_dist(0.0f, 6.2831853f);
  std::uniform_real_distribution<float> offset_dist(-18.0f, 18.0f);

  for (const SpawnRing& ring : rings) {
    std::uniform_real_distribution<float> distance_dist(ring.min_distance,
                                                        ring.max_distance);
    for (int i = 0; i < ring.count; ++i) {
      glm::vec3 spawn_pos(0.0f);
      bool found_spawn = false;
      for (int attempt = 0; attempt < 64 && !found_spawn; ++attempt) {
        const float angle = angle_dist(rng);
        const float distance = distance_dist(rng);
        const float x = player_pos.x + std::cos(angle) * distance;
        const float z = player_pos.z + std::sin(angle) * distance;

        if (x < m_city_world_min.x + CITY_INSET ||
            x > m_city_world_max.x - CITY_INSET ||
            z < m_city_world_min.z + CITY_INSET ||
            z > m_city_world_max.z - CITY_INSET)
          continue;

        float ground = 0.0f;
        if (!m_collision_world.find_ground_height(x, z, 6.0f, -2.0f, ground))
          continue;
        if (m_collision_world.blocks_vertical_capsule(
              x, z, x, z, ground + 0.15f, ground + 1.75f, 0.4f))
          continue;

        spawn_pos = glm::vec3(x, ground, z);
        found_spawn = true;
      }
      if (!found_spawn)
        continue;

      const float ox = offset_dist(rng);
      const float oz = offset_dist(rng);
      std::vector<glm::vec3> patrol_route = { spawn_pos };
      // Skip patrol legs that would run the enemy into a wall.
      const glm::vec3 leg_a = spawn_pos + glm::vec3(ox, 0.0f, oz);
      const glm::vec3 leg_b = spawn_pos - glm::vec3(ox, 0.0f, oz);
      if (is_walkable(leg_a))
        patrol_route.push_back(leg_a);
      if (is_walkable(leg_b))
        patrol_route.push_back(leg_b);
      m_enemies.push_back(
        std::make_shared<Enemy>(*this, spawn_pos, patrol_route));
    }
  }

  if (!m_enemies.empty()) {
    float nearest = std::numeric_limits<float>::max();
    for (const auto& enemy : m_enemies) {
      const glm::vec3 enemy_pos = enemy->get_position();
      nearest = std::min(nearest,
                         glm::distance(glm::vec2(enemy_pos.x, enemy_pos.z),
                                       glm::vec2(player_pos.x, player_pos.z)));
    }
    std::fprintf(stderr,
                 "Rival Grounds: %zu enemies spawned, nearest %.1f m away.\n",
                 m_enemies.size(),
                 nearest);
  }
}

void
RivalGrounds::spawn_physics_obj_prop(const std::string& filename,
                                     const glm::vec3& pos,
                                     const glm::vec3& scale,
                                     const glm::vec3& color)
{
  auto obj = std::make_shared<SceneObject>();
  obj->get_local_transform().set_position(pos).set_scale(scale);

  obj->get_physics_component().enable();
  obj->get_physics_component().set_mass(2.5f).set_gravity(true);
  obj->set_collidable(true);

  obj->set_render_packet(
    m_app_cache.mesh_repo.load_mesh(
      from_OBJ<Vertex_PosTexNorm, TriangleIndices>(filename, GL_TRIANGLES)),
    m_app_cache.material_repo.load_material(MaterialDescriptor{
      .material_name = "phys_mat_" + filename + "_" + std::to_string(pos.x) +
                       "_" + std::to_string(pos.z),
      .shader_program_desc = { .program_name = "cube_program",
                               .shaders = { "cube.vert", "cube.frag" } },
      .texture_desc_list = {},
      .uniforms = { { "u_color", color },
                    { "u_use_texture", 0 },
                    { "u_is_gun", 0 } } }),
    capture_weak(
      obj, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
        set_view_matrix(packet);
        set_projection_matrix(packet);
        set_model_matrix(packet, self->get_world_transformation_mat());
        packet->material->get_shader_program()->set_uniform("u_preserve_color",
                                                            1);
        packet->mesh->draw();
        packet->material->get_shader_program()->set_uniform("u_preserve_color",
                                                            0);
      }));

  m_scene_ptr->add_child(obj);
}

void
RivalGrounds::setup_plane()
{
  auto plane = std::make_shared<SceneObject>();
  plane->set_collidable(false);

  const float terrain_width = m_city_world_max.x - m_city_world_min.x;
  const float terrain_depth = m_city_world_max.z - m_city_world_min.z;

  const glm::vec3 terrain_center(
    (m_city_world_min.x + m_city_world_max.x) * 0.5f,
    -0.02f,
    (m_city_world_min.z + m_city_world_max.z) * 0.5f);

  plane->get_local_transform()
    .set_position(terrain_center)
    .set_scale({ terrain_width, 1.0f, terrain_depth });
  plane->get_physics_component().disable();

  std::vector<Vertex_PosTexNorm> verts = {
    { { -0.5f, 0.0f, -0.5f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.0f, -0.5f }, { 120.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.0f, 0.5f }, { 120.0f, 120.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, 0.0f, 0.5f }, { 0.0f, 120.0f }, { 0.0f, 1.0f, 0.0f } }
  };
  std::vector<TriangleIndices> inds = { { 0, 1, 2 }, { 0, 2, 3 } };

  MeshDescriptor<Vertex_PosTexNorm, TriangleIndices> plane_desc{
    "city_under_plane_mesh_flush",
    std::move(verts),
    std::move(inds),
    GL_TRIANGLES
  };

  plane->set_render_packet(
    m_app_cache.mesh_repo.load_mesh(plane_desc),
    m_app_cache.material_repo.load_material(MaterialDescriptor{
      .material_name = "sub_ground_concrete_mat",
      .shader_program_desc = { .program_name = "cube_program",
                               .shaders = { "cube.vert", "cube.frag" } },
      .texture_desc_list = { { .texture_name = "concrete",
                               .wrap_s = GL_REPEAT,
                               .wrap_t = GL_REPEAT } },
      .uniforms = { { "u_diffuse", TextureName("concrete") },
                    { "u_color", glm::vec3(0.38f, 0.38f, 0.40f) },
                    { "u_use_texture", 1 },
                    { "u_is_gun", 0 } } }),
    capture_weak(
      plane, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
        set_view_matrix(packet);
        set_projection_matrix(packet);
        set_model_matrix(packet, self->get_world_transformation_mat());
        packet->mesh->draw();
      }));

  m_scene_ptr->add_child(plane);
}

MeshDescriptor<Vertex_PosTexNorm, TriangleIndices>
RivalGrounds::get_sharp_box_descriptor()
{
  std::vector<Vertex_PosTexNorm> vertices;
  std::vector<TriangleIndices> indices;

  auto add_face =
    [&](glm::vec3 n, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
      GLuint base = static_cast<GLuint>(vertices.size());
      vertices.push_back({ p0, { 0.f, 0.f }, n });
      vertices.push_back({ p1, { 1.f, 0.f }, n });
      vertices.push_back({ p2, { 1.f, 1.f }, n });
      vertices.push_back({ p3, { 0.f, 1.f }, n });
      indices.push_back({ base, base + 1, base + 2 });
      indices.push_back({ base, base + 2, base + 3 });
    };

  add_face({ 0, 1, 0 },
           { -0.5, 0.5, -0.5 },
           { -0.5, 0.5, 0.5 },
           { 0.5, 0.5, 0.5 },
           { 0.5, 0.5, -0.5 });
  add_face({ 0, -1, 0 },
           { -0.5, -0.5, 0.5 },
           { -0.5, -0.5, -0.5 },
           { 0.5, -0.5, -0.5 },
           { 0.5, -0.5, 0.5 });
  add_face({ 0, 0, 1 },
           { -0.5, -0.5, 0.5 },
           { 0.5, -0.5, 0.5 },
           { 0.5, 0.5, 0.5 },
           { -0.5, 0.5, 0.5 });
  add_face({ 0, 0, -1 },
           { 0.5, -0.5, -0.5 },
           { -0.5, -0.5, -0.5 },
           { -0.5, 0.5, -0.5 },
           { 0.5, 0.5, -0.5 });
  add_face({ -1, 0, 0 },
           { -0.5, -0.5, -0.5 },
           { -0.5, -0.5, 0.5 },
           { -0.5, 0.5, 0.5 },
           { -0.5, 0.5, -0.5 });
  add_face({ 1, 0, 0 },
           { 0.5, -0.5, 0.5 },
           { 0.5, -0.5, -0.5 },
           { 0.5, 0.5, -0.5 },
           { 0.5, 0.5, 0.5 });

  return {
    "sharp_box_mesh", std::move(vertices), std::move(indices), GL_TRIANGLES
  };
}
