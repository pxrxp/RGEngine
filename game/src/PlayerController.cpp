#include <rivalgrounds/RivalGrounds.h>

#include <engine/repos/MeshRepo.h>
#include <engine/util/obj_model.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <random>

void
RivalGrounds::update_physics()
{
  auto& window = m_app_cache.get_window();
  if (window.is_key_pressed(GLFW_KEY_ESCAPE))
    window.set_cursor_mode(GLFW_CURSOR_NORMAL);
  if (window.is_key_pressed(GLFW_KEY_TAB))
    window.set_cursor_mode(GLFW_CURSOR_DISABLED);

  if (m_game_state != GameState::PLAYING) {
    if (m_app_cache.get_window().is_key_pressed(GLFW_KEY_R)) {
      reset_match();
      return;
    }
    if (m_game_state == GameState::INTRO) {
      if (window.is_key_pressed(GLFW_KEY_SPACE) ||
          window.is_key_pressed(GLFW_KEY_ENTER) ||
          window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_game_state = GameState::PLAYING;
        m_spawn_grace_timer = 10.0f;
      }
    }
    return;
  }

  Scene::update_physics();
  float dt = Scene::FIXED_STEP;

  if (m_gun_recoil > 0.001f) {
    m_gun_recoil -= m_gun_recoil * 18.0f * dt;
  } else {
    m_gun_recoil = 0.0f;
  }

  if (m_damage_flash_timer > 0.0f)
    m_damage_flash_timer -= dt;

  if (m_hitmarker_timer > 0.0f)
    m_hitmarker_timer -= dt;

  if (m_spawn_grace_timer > 0.0f)
    m_spawn_grace_timer -= dt;

  if (m_player_regen_cooldown > 0.0f) {
    m_player_regen_cooldown -= dt;
  } else if (m_player_health < m_player_max_health) {
    m_player_health =
      std::min(m_player_max_health, m_player_health + 4.0f * dt);
  }

  for (auto it = m_tracers.begin(); it != m_tracers.end();) {
    it->lifetime -= dt;
    if (it->lifetime <= 0.0f) {
      it = m_tracers.erase(it);
    } else {
      ++it;
    }
  }

#ifndef NDEBUG
  const bool f1_pressed = m_app_cache.get_window().is_key_pressed(GLFW_KEY_F1);
  if (f1_pressed && !m_debug_f1_was_pressed) {
    m_debug_menu_open = !m_debug_menu_open;
    m_app_cache.get_window().set_cursor_mode(
      m_debug_menu_open ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
  }
  m_debug_f1_was_pressed = f1_pressed;
#endif

  update_character_controller(dt);
  handle_shooting();
  handle_object_grab();

  glm::vec3 player_pos = m_player->get_local_transform().get_position();
  glm::vec3 cam_fwd = m_app_cache.get_camera().get_forward();

  std::vector<BoundingBox> colliders;
  collect_game_colliders(colliders);

  cull_distant_enemies();

  m_reinforcement_timer -= dt;
  if (m_reinforcement_timer <= 0.0f) {
    m_reinforcement_timer = 14.0f;

    size_t living = 0;
    for (const auto& e : m_enemies) {
      if (!e->is_dead())
        ++living;
    }

    if (living < 14)
      spawn_enemy_near_player(45.0f, 75.0f);
  }

  for (auto& enemy : m_enemies) {
    if (enemy->is_dead())
      continue;

    float damage = 0.0f;
    bool shot_fired = false;
    glm::vec3 gun_pos(0.0f);
    enemy->update(
      *this, player_pos, cam_fwd, colliders, dt, damage, shot_fired, gun_pos);

    float shot_distance = glm::distance(gun_pos, player_pos);
    if (shot_fired && shot_distance > 0.001f) {
      const glm::vec3 shot_direction = (player_pos - gun_pos) / shot_distance;
      float obstruction_distance = shot_distance;
      float mesh_distance = 0.0f;
      bool blocked = static_mesh_ray_hit(
        gun_pos, shot_direction, shot_distance, mesh_distance);
      if (blocked)
        obstruction_distance = mesh_distance;

      for (const auto& aabb : colliders) {
        float t = 0.0f;
        glm::vec3 normal(0.0f);
        if (aabb.intersects_ray(gun_pos, shot_direction, t, normal) &&
            t > 0.05f && t < obstruction_distance) {
          obstruction_distance = t;
          blocked = true;
        }
      }

      if (blocked && obstruction_distance < shot_distance - 0.15f)
        damage = 0.0f;
    }

    if (shot_fired) {
      glm::vec3 tracer_end = player_pos + glm::vec3(0, -0.2f, 0);
      const float tracer_distance = glm::distance(gun_pos, tracer_end);
      if (tracer_distance > 0.001f) {
        const glm::vec3 tracer_direction =
          (tracer_end - gun_pos) / tracer_distance;
        float obstruction_distance = tracer_distance;
        float mesh_distance = 0.0f;
        if (static_mesh_ray_hit(
              gun_pos, tracer_direction, tracer_distance, mesh_distance))
          obstruction_distance = mesh_distance;
        for (const auto& aabb : colliders) {
          float t = 0.0f;
          glm::vec3 normal(0.0f);
          if (aabb.intersects_ray(gun_pos, tracer_direction, t, normal) &&
              t > 0.05f && t < obstruction_distance)
            obstruction_distance = t;
        }
        if (obstruction_distance < tracer_distance)
          tracer_end = gun_pos + tracer_direction * obstruction_distance;
      }
      m_tracers.push_back(
        { gun_pos, tracer_end, 0.12f, glm::vec3(1.0f, 0.3f, 0.2f) });
    }

#ifndef NDEBUG
    if (!m_debug_invincible) {
#endif
      if (damage > 0.0f && m_spawn_grace_timer <= 0.0f) {
        m_player_health -= damage;
        m_player_regen_cooldown = 6.0f;
        m_damage_flash_timer = 0.28f;
        if (m_player_health <= 0.0f) {
          m_player_health = 0.0f;
          m_game_state = GameState::DEFEATED;
          return;
        }
      }
#ifndef NDEBUG
    }
#endif
  }

  float dist_to_scroll =
    glm::distance(glm::vec2(player_pos.x, player_pos.z),
                  glm::vec2(m_scroll_pos.x, m_scroll_pos.z));
  if (dist_to_scroll < 2.2f && std::abs(player_pos.y - m_scroll_pos.y) < 2.0f) {
    m_game_state = GameState::VICTORY;
    m_scroll_objective->visible = false;
  }
}

bool
RivalGrounds::has_open_ceiling(const glm::vec3& pos)
{
  const glm::vec3 ray_origin = pos + glm::vec3(0.0f, 0.5f, 0.0f);
  const glm::vec3 up_dir(0.0f, 1.0f, 0.0f);
  const float check_distance = 150.0f;
  float out_dist = 0.0f;

  if (static_mesh_ray_hit(ray_origin, up_dir, check_distance, out_dist)) {
    return false;
  }

  std::vector<BoundingBox> colliders;
  collect_game_colliders(colliders, false);
  for (const auto& aabb : colliders) {
    float t = 0.0f;
    glm::vec3 norm(0.0f);
    if (aabb.intersects_ray(ray_origin, up_dir, t, norm) && t > 0.1f) {
      return false;
    }
  }

  return true;
}

bool
RivalGrounds::blocks_vertical_capsule(float x,
                                      float z,
                                      float from_x,
                                      float from_z,
                                      float feet_y,
                                      float head_y,
                                      float radius) const
{
  return m_collision_world.blocks_vertical_capsule(
    x, z, from_x, from_z, feet_y, head_y, radius);
}

bool
RivalGrounds::find_ground_surface_for_actor(float x,
                                            float z,
                                            float max_y,
                                            float min_y,
                                            float& out_y) const
{
  return m_collision_world.find_ground_height(x, z, max_y, min_y, out_y);
}

bool
RivalGrounds::find_ground_surface(float x,
                                  float z,
                                  float max_y,
                                  float min_y,
                                  float& out_y) const
{
  return m_collision_world.find_ground_height(x, z, max_y, min_y, out_y);
}

bool
RivalGrounds::find_landing_surface(float x,
                                   float z,
                                   float radius,
                                   float previous_bottom,
                                   float current_bottom,
                                   float& out_y) const
{
  return m_collision_world.find_landing_surface(
    x, z, radius, previous_bottom, current_bottom, 0.45f, out_y);
}

bool
RivalGrounds::static_mesh_horizontal_blocked(float x,
                                             float z,
                                             float from_x,
                                             float from_z,
                                             float feet_y,
                                             float head_y) const
{
  return m_collision_world.blocks_vertical_capsule(
    x, z, from_x, from_z, feet_y, head_y, PLAYER_RADIUS);
}

bool
RivalGrounds::static_mesh_ray_hit(const glm::vec3& origin,
                                  const glm::vec3& direction,
                                  float max_distance,
                                  float& out_distance) const
{
  return m_collision_world.raycast(
    origin, direction, max_distance, out_distance);
}

void
RivalGrounds::collect_game_colliders(std::vector<BoundingBox>& colliders,
                                     bool include_nearby_enemies)
{
  colliders.clear();
  collect_colliders_recursive(m_scene_ptr, colliders);

  if (!include_nearby_enemies)
    return;

  for (const auto& enemy : m_enemies) {
    if (enemy && !enemy->is_dead())
      colliders.push_back(enemy->get_hitbox());
  }
}

void
RivalGrounds::update_character_controller(float dt)
{
  if (m_app_cache.get_window().get_cursor_mode() != GLFW_CURSOR_DISABLED)
    return;

  if (m_jump_cooldown > 0.0f)
    m_jump_cooldown -= dt;

  auto& window = m_app_cache.get_window();

  std::vector<BoundingBox> colliders;
#ifndef NDEBUG
  if (!m_debug_noclip)
#endif
    collect_game_colliders(colliders, false);

  glm::vec3 player_pos = m_player->get_local_transform().get_position();
  float feet_y = player_pos.y - PLAYER_EYE_HEIGHT;
  const float head_y = feet_y + 1.65f;

  glm::vec3 wish(0.0f);
  if (window.is_key_pressed(GLFW_KEY_W) || window.is_key_pressed(GLFW_KEY_UP))
    wish.z += 1.0f;
  if (window.is_key_pressed(GLFW_KEY_S) || window.is_key_pressed(GLFW_KEY_DOWN))
    wish.z -= 1.0f;
  if (window.is_key_pressed(GLFW_KEY_A) || window.is_key_pressed(GLFW_KEY_LEFT))
    wish.x -= 1.0f;
  if (window.is_key_pressed(GLFW_KEY_D) ||
      window.is_key_pressed(GLFW_KEY_RIGHT))
    wish.x += 1.0f;

  float speed = 7.0f;
  if (window.is_key_pressed(GLFW_KEY_LEFT_CONTROL)) {
    speed = 11.5f;
  } else if (window.is_key_pressed(GLFW_KEY_LEFT_SHIFT)) {
    speed = 3.8f;
  }

  glm::quat rot = m_player->get_local_transform().get_rotation();
  glm::vec3 fwd_raw = rot * glm::vec3(AXIS_NEG_Z);
  glm::vec3 rgt_raw = rot * glm::vec3(AXIS_X);

  glm::vec3 fwd = glm::normalize(glm::vec3(fwd_raw.x, 0.0f, fwd_raw.z));
  glm::vec3 rgt = glm::normalize(glm::vec3(rgt_raw.x, 0.0f, rgt_raw.z));

  glm::vec3 move_dir = rgt * wish.x + fwd * wish.z;
  if (glm::dot(move_dir, move_dir) > 0.0f)
    move_dir = glm::normalize(move_dir);
#ifndef NDEBUG
  if (m_debug_noclip) {
    float vertical = 0.0f;
    if (window.is_key_pressed(GLFW_KEY_E) ||
        window.is_key_pressed(GLFW_KEY_SPACE))
      vertical += 1.0f;
    if (window.is_key_pressed(GLFW_KEY_Q) ||
        window.is_key_pressed(GLFW_KEY_LEFT_SHIFT))
      vertical -= 1.0f;
    player_pos += move_dir * speed * dt;
    player_pos.y += vertical * speed * dt;
    m_player_vy = 0.0f;
    m_player_grounded = false;
    m_player->get_local_transform().set_position(player_pos);
    m_app_cache.get_camera().set_view_matrix_dirty();
    return;
  }
#endif

  if (glm::dot(move_dir, move_dir) > 0.0f) {
    glm::vec3 delta = move_dir * speed * dt;

    auto test_axis = [&](float nx, float nz) -> bool {
      for (const auto& aabb : colliders) {
        const glm::vec3& b_min = aabb.get_min();
        const glm::vec3& b_max = aabb.get_max();
        if (b_min.y >= head_y - 0.02f || b_max.y <= feet_y + 0.02f)
          continue;

        const float closest_x = glm::clamp(nx, b_min.x, b_max.x);
        const float closest_z = glm::clamp(nz, b_min.z, b_max.z);
        const float dx = nx - closest_x;
        const float dz = nz - closest_z;
        if ((dx * dx + dz * dz) >= PLAYER_RADIUS * PLAYER_RADIUS)
          continue;

        const float current_closest_x =
          glm::clamp(player_pos.x, b_min.x, b_max.x);
        const float current_closest_z =
          glm::clamp(player_pos.z, b_min.z, b_max.z);
        const float current_dx = player_pos.x - current_closest_x;
        const float current_dz = player_pos.z - current_closest_z;
        const bool current_overlaps =
          current_dx * current_dx + current_dz * current_dz <
          PLAYER_RADIUS * PLAYER_RADIUS;
        if (current_overlaps) {
          const auto penetration = [&](float px, float pz) {
            return std::min({ px + PLAYER_RADIUS - b_min.x,
                              b_max.x - (px - PLAYER_RADIUS),
                              pz + PLAYER_RADIUS - b_min.z,
                              b_max.z - (pz - PLAYER_RADIUS) });
          };
          if (penetration(nx, nz) <
              penetration(player_pos.x, player_pos.z) - 0.001f)
            continue;
        }
        return false;
      }
      return !static_mesh_horizontal_blocked(
        nx, nz, player_pos.x, player_pos.z, feet_y, head_y);
    };

    if (test_axis(player_pos.x + delta.x, player_pos.z))
      player_pos.x += delta.x;
    if (test_axis(player_pos.x, player_pos.z + delta.z))
      player_pos.z += delta.z;
  }

  bool space = window.is_key_pressed(GLFW_KEY_SPACE);
  if (space && !m_space_was_pressed && m_player_grounded &&
      m_jump_cooldown <= 0.0f) {
    m_player_vy = 7.2f;
    m_player_grounded = false;
    m_jump_cooldown = 0.25f;
  }
  m_space_was_pressed = space;

  const float previous_feet_y = feet_y;
  m_player_vy -= 18.0f * dt;
  player_pos.y += m_player_vy * dt;
  feet_y = player_pos.y - PLAYER_EYE_HEIGHT;

  if (m_player_vy <= 0.0f) {
    float landing_y = -std::numeric_limits<float>::infinity();

    if (previous_feet_y >= -0.05f && feet_y <= 0.0f) {
      landing_y = 0.0f;
    }
    auto consider_landing = [&](const BoundingBox& aabb) {
      const glm::vec3& b_min = aabb.get_min();
      const glm::vec3& b_max = aabb.get_max();
      const float closest_x = glm::clamp(player_pos.x, b_min.x, b_max.x);
      const float closest_z = glm::clamp(player_pos.z, b_min.z, b_max.z);
      const float dx = player_pos.x - closest_x;
      const float dz = player_pos.z - closest_z;
      const bool horizontal_overlap =
        dx * dx + dz * dz < PLAYER_RADIUS * PLAYER_RADIUS;
      const bool crossed_top =
        previous_feet_y >= b_max.y - 0.025f && feet_y <= b_max.y;
      if (horizontal_overlap && crossed_top && b_max.y > landing_y)
        landing_y = b_max.y;
    };
    for (const auto& collider : colliders)
      consider_landing(collider);

    float mesh_landing_y = 0.0f;
    if (m_collision_world.find_landing_surface(player_pos.x,
                                               player_pos.z,
                                               PLAYER_RADIUS,
                                               previous_feet_y,
                                               feet_y,
                                               0.45f,
                                               mesh_landing_y) &&
        mesh_landing_y > landing_y)
      landing_y = mesh_landing_y;

    if (std::isfinite(landing_y)) {
      player_pos.y = landing_y + PLAYER_EYE_HEIGHT;
      m_player_vy = 0.0f;
      if (!m_player_grounded && m_awaiting_landing) {
        // The drop from the rooftops takes longer than the initial grace, so
        // the player would land defenceless. Start the grace on touchdown.
        m_awaiting_landing = false;
        m_spawn_grace_timer = 10.0f;
      }
      m_player_grounded = true;
    } else {
      m_player_grounded = false;
    }
  }

  if (m_city_bounds_valid) {
    player_pos.x = glm::clamp(player_pos.x,
                              m_city_world_min.x + PLAYER_RADIUS,
                              m_city_world_max.x - PLAYER_RADIUS);

    player_pos.z = glm::clamp(player_pos.z,
                              m_city_world_min.z + PLAYER_RADIUS,
                              m_city_world_max.z - PLAYER_RADIUS);
  }

  m_player->get_local_transform().set_position(player_pos);
  m_app_cache.get_camera().set_view_matrix_dirty();
}
