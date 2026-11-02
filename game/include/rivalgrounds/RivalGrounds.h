#pragma once
#include <engine/App.h>
#include <engine/CollisionWorld.h>
#include <rivalgrounds/Enemy.h>
#include <engine/Material.h>
#include <engine/Mesh.h>
#include <engine/Renderer.h>
#include <engine/Scene.h>
#include <engine/SceneObject.h>
#include <engine/ShaderProgram.h>
#include <engine/Texture.h>
#include <engine/components/vertex_formats.h>
#include <engine/repos/MaterialRepo.h>
#include <engine/repos/MeshRepo.h>
#include <engine/util/obj_model.h>
#include <imgui.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <random>
#include <unordered_map>

enum class GameState
{
  INTRO,
  PLAYING,
  VICTORY,
  DEFEATED
};

struct BulletTracer
{
  glm::vec3 start;
  glm::vec3 end;
  float lifetime;
  glm::vec3 color;
};

class RivalGrounds : public Scene
{
private:
  GameState m_game_state = GameState::INTRO;
  TextureStrongPtr m_logo_texture = nullptr;

  SceneObjectStrongPtr m_player;
  float m_player_health = 100.0f;
  float m_player_max_health = 100.0f;
  float m_player_regen_cooldown = 0.0f;
  float m_player_vy = 0.0f;
  bool m_player_grounded = true;
  bool m_space_was_pressed = false;
  float m_jump_cooldown = 0.0f;
  float m_spawn_grace_timer = 2.0f;
  float m_reinforcement_timer = 2.0f;

  static constexpr float PLAYER_EYE_HEIGHT = 1.7f;
  static constexpr float PLAYER_RADIUS = 0.28f;

  const glm::vec3 m_player_spawn = { -11.58f, PLAYER_EYE_HEIGHT, -0.48f };
  glm::vec3 m_scroll_pos = { 0.0f, 0.8f, -100.0f };

  SceneObjectStrongPtr m_weapon_viewmodel;
  float m_gun_recoil = 0.0f;
  bool m_mouse_was_pressed = false;
  bool m_right_mouse_was_pressed = false;
  float m_damage_flash_timer = 0.0f;
  float m_hitmarker_timer = 0.0f;
  std::vector<BulletTracer> m_tracers;

  SceneObjectStrongPtr m_held_object = nullptr;
  float m_held_object_hide_cooldown = 2.0f;

  SceneObjectStrongPtr m_scroll_objective;
  std::vector<std::shared_ptr<Enemy>> m_enemies;
  CollisionWorld m_collision_world;

  glm::vec3 m_city_world_min{ 0.0f };
  glm::vec3 m_city_world_max{ 0.0f };
  bool m_city_bounds_valid = false;

#ifndef NDEBUG
  bool m_debug_menu_open = false;
  bool m_debug_noclip = false;
  bool m_debug_show_hitboxes = false;
  bool m_debug_f1_was_pressed = false;
  bool m_debug_invincible = false;
#endif

#ifndef NDEBUG
  void render_debug_menu();
#endif

public:
  RivalGrounds(App& app)
    : Scene(app)
  {
  }

  void init() override
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

  bool has_open_ceiling(const glm::vec3& pos)
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

  bool blocks_vertical_capsule(float x,
                               float z,
                               float from_x,
                               float from_z,
                               float feet_y,
                               float head_y,
                               float radius) const override
  {
    return m_collision_world.blocks_vertical_capsule(
      x, z, from_x, from_z, feet_y, head_y, radius);
  }

  bool find_ground_surface_for_actor(float x,
                                     float z,
                                     float max_y,
                                     float min_y,
                                     float& out_y) const override
  {
    return m_collision_world.find_ground_height(x, z, max_y, min_y, out_y);
  }

  bool find_ground_surface(float x,
                           float z,
                           float max_y,
                           float min_y,
                           float& out_y) const override
  {
    return m_collision_world.find_ground_height(x, z, max_y, min_y, out_y);
  }

  bool find_landing_surface(float x,
                            float z,
                            float radius,
                            float previous_bottom,
                            float current_bottom,
                            float& out_y) const override
  {
    return m_collision_world.find_landing_surface(
      x, z, radius, previous_bottom, current_bottom, 0.45f, out_y);
  }

  void setup_weapon_viewmodel()
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
            glm::rotate(
              glm::mat4(1.0f), glm::radians(6.0f), glm::vec3(0, 0, 1)) *
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

          packet->material->get_shader_program()->set_uniform(
            "u_preserve_color", 1);
          packet->mesh->draw();
          packet->material->get_shader_program()->set_uniform(
            "u_preserve_color", 0);
        }));

    if (auto pkt = m_weapon_viewmodel->get_render_packet()) {
      pkt.value()->priority = LOW_PRIORITY;
    }
  }

  void update_physics() override
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
          m_spawn_grace_timer = 2.5f;
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

    if (m_held_object) {
      glm::vec3 hold_pos = m_player->get_local_transform().get_position() +
                           m_app_cache.get_camera().get_forward() * 2.4f +
                           glm::vec3(0.0f, -0.1f, 0.0f);
      m_held_object->get_local_transform().set_position(hold_pos).set_rotation(
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      m_held_object->get_physics_component().set_linear_momentum({ 0, 0, 0 });
      m_held_object->get_physics_component().set_angular_momentum({ 0, 0, 0 });
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
    const bool f1_pressed =
      m_app_cache.get_window().is_key_pressed(GLFW_KEY_F1);
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

    m_reinforcement_timer -= dt;
    if (m_reinforcement_timer <= 0.0f) {
      m_reinforcement_timer = 14.0f;

      size_t living = 0;
      for (const auto& e : m_enemies) {
        if (!e->is_dead())
          ++living;
      }

      if (living < 14) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_real_distribution<float> angle_dist(0.0f, 6.2831853f);
        std::uniform_real_distribution<float> dist_dist(55.0f, 115.0f);

        float ang = angle_dist(rng);
        float d = dist_dist(rng);
        glm::vec3 spawn_pos(player_pos.x + std::cos(ang) * d,
                            0.0f,
                            player_pos.z + std::sin(ang) * d);

        std::vector<glm::vec3> patrol_route = {
          spawn_pos + glm::vec3(25.0f, 0.0f, 0.0f),
          spawn_pos - glm::vec3(25.0f, 0.0f, 0.0f)
        };
        m_enemies.push_back(
          std::make_shared<Enemy>(*this, spawn_pos, patrol_route));
      }
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
    if (dist_to_scroll < 2.2f &&
        std::abs(player_pos.y - m_scroll_pos.y) < 2.0f) {
      m_game_state = GameState::VICTORY;
      m_scroll_objective->visible = false;
    }
  }

  void submit_to_renderer()
  {
    Scene::submit_to_renderer();
    render_game_hud();
  }

  bool static_mesh_horizontal_blocked(float x,
                                      float z,
                                      float from_x,
                                      float from_z,
                                      float feet_y,
                                      float head_y) const
  {
    return m_collision_world.blocks_vertical_capsule(
      x, z, from_x, from_z, feet_y, head_y, PLAYER_RADIUS);
  }

  bool static_mesh_ray_hit(const glm::vec3& origin,
                           const glm::vec3& direction,
                           float max_distance,
                           float& out_distance) const override
  {
    return m_collision_world.raycast(
      origin, direction, max_distance, out_distance);
  }

private:
  void collect_game_colliders(std::vector<BoundingBox>& colliders,
                              bool include_nearby_enemies = true)
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

  void update_character_controller(float dt)
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
    if (window.is_key_pressed(GLFW_KEY_S) ||
        window.is_key_pressed(GLFW_KEY_DOWN))
      wish.z -= 1.0f;
    if (window.is_key_pressed(GLFW_KEY_A) ||
        window.is_key_pressed(GLFW_KEY_LEFT))
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

    if (m_held_object && window.is_key_pressed(GLFW_KEY_H)) {
      if (m_held_object_hide_cooldown >= 0) {
        m_held_object_hide_cooldown -= 0.4f;
      } else {
        m_held_object->visible = !m_held_object->visible;
        m_held_object_hide_cooldown = 2.0f;
      }
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

  void handle_object_grab()
  {
    auto& window = m_app_cache.get_window();
    bool right_pressed =
      window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);

    if (right_pressed && !m_right_mouse_was_pressed) {
      if (m_held_object) {
        glm::vec3 origin = m_player->get_local_transform().get_position();
        glm::vec3 dir = m_app_cache.get_camera().get_forward();

        const float CRATE_SIZE = 1.80f;
        const float CRATE_HALF = 0.90f;

        auto objects = get_all_physics_objects();
        float closest_t = 4.2f;
        SceneObjectStrongPtr hit_box = nullptr;
        glm::vec3 hit_norm(0.0f, 1.0f, 0.0f);

        for (auto& obj : objects) {
          if (obj == m_held_object)
            continue;
          float t = 0.0f;
          glm::vec3 norm(0.0f);
          if (obj->get_world_AABB().intersects_ray(origin, dir, t, norm)) {
            if (t > 0.2f && t < closest_t) {
              closest_t = t;
              hit_box = obj;
              hit_norm = norm;
            }
          }
        }

        glm::vec3 place_pos(0.0f);

        if (hit_box) {

          glm::vec3 base_pos = hit_box->get_local_transform().get_position();
          if (hit_norm.y > 0.4f) {

            place_pos = base_pos + glm::vec3(0.0f, CRATE_SIZE, 0.0f);
          } else {

            glm::vec3 flat_norm =
              glm::normalize(glm::vec3(hit_norm.x, 0.0f, hit_norm.z));
            place_pos = base_pos + flat_norm * CRATE_SIZE;
            place_pos.y = base_pos.y;
          }
        } else {

          place_pos = origin + dir * 2.2f;
          place_pos.y = CRATE_HALF;
        }

        m_held_object->get_local_transform()
          .set_position(place_pos)
          .set_rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

        m_held_object->set_collidable(true);
        auto& phys = m_held_object->get_physics_component();
        phys.enable();
        phys.set_gravity(true);
        phys.set_linear_momentum({ 0.0f, 0.0f, 0.0f });
        phys.set_angular_momentum({ 0.0f, 0.0f, 0.0f });

        m_held_object = nullptr;
      } else {

        glm::vec3 origin = m_player->get_local_transform().get_position();
        glm::vec3 dir = m_app_cache.get_camera().get_forward();

        auto objects = get_all_physics_objects();
        SceneObjectStrongPtr closest_obj = nullptr;
        float closest_dist = 4.0f;

        for (auto& obj : objects) {
          float t = 0.0f;
          glm::vec3 norm(0.0f);
          if (obj->get_world_AABB().intersects_ray(origin, dir, t, norm)) {
            if (t > 0.2f && t < closest_dist) {
              closest_dist = t;
              closest_obj = obj;
            }
          }
        }

        if (closest_obj) {
          m_held_object = closest_obj;

          m_held_object->set_collidable(false);
          m_held_object->get_physics_component().disable();
        }
      }
    }
    m_right_mouse_was_pressed = right_pressed;

    if (m_held_object) {
      glm::vec3 hold_pos = m_player->get_local_transform().get_position() +
                           m_app_cache.get_camera().get_forward() * 2.2f +
                           glm::vec3(0.0f, -0.1f, 0.0f);
      m_held_object->get_local_transform().set_position(hold_pos).set_rotation(
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      m_held_object->visible = true;
    }
  }

  void handle_shooting()
  {
    auto& window = m_app_cache.get_window();
    bool is_pressed = window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);

    if (is_pressed && !m_mouse_was_pressed) {
      glm::vec3 origin = m_player->get_local_transform().get_position();
      glm::vec3 dir = m_app_cache.get_camera().get_forward();

      m_gun_recoil = 0.11f;

      std::vector<BoundingBox> colliders;
      collect_game_colliders(colliders, false);

      float env_t = 200.0f;
      float mesh_t = 0.0f;
      if (static_mesh_ray_hit(origin, dir, env_t, mesh_t))
        env_t = mesh_t;
      for (const auto& aabb : colliders) {
        float t = 0.0f;
        glm::vec3 norm(0.0f);
        if (aabb.intersects_ray(origin, dir, t, norm)) {
          if (t > 0.2f && t < env_t) {
            env_t = t;
          }
        }
      }

      bool hit_enemy = false;
      for (auto& enemy : m_enemies) {
        if (!enemy->is_dead()) {
          float t = 0.0f;
          glm::vec3 norm(0.0f);
          if (enemy->get_hitbox().intersects_ray(origin, dir, t, norm)) {
            if (t < env_t) {
              env_t = t;
              m_hitmarker_timer = 0.22f;
              enemy->take_damage(*this, 65.0f, dir * 0.4f, colliders);
              m_tracers.push_back({ origin + dir * 0.4f,
                                    origin + dir * t,
                                    0.08f,
                                    glm::vec3(0.9f, 0.8f, 0.3f) });
              hit_enemy = true;
            }
          }
        }
      }

      if (!hit_enemy) {
        m_tracers.push_back({ origin + dir * 0.4f,
                              origin + dir * env_t,
                              0.08f,
                              glm::vec3(0.9f, 0.8f, 0.3f) });
      }

      auto objects = get_all_physics_objects();
      for (auto& obj : objects) {
        float t = 0.0f;
        glm::vec3 normal(0.0f);
        if (obj->get_world_AABB().intersects_ray(origin, dir, t, normal)) {
          if (t <= env_t) {
            glm::vec3 hit_pt = origin + dir * t;
            obj->get_physics_component().apply_impulse_at_point(
              dir * 8.0f + glm::vec3(0, 1.2f, 0), hit_pt);
            obj->get_physics_component().set_angular_momentum(
              glm::vec3(0.4f, 0.6f, 0.2f));
          }
        }
      }
    }
    m_mouse_was_pressed = is_pressed;
  }

  void render_game_hud()
  {
#ifndef NDEBUG
    render_debug_menu();
#endif
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    const ImGuiIO& io = ImGui::GetIO();
    float cx = io.DisplaySize.x * 0.5f;
    float cy = io.DisplaySize.y * 0.5f;

    if (m_game_state == GameState::INTRO) {
      draw->AddRectFilled(
        ImVec2(0, 0), io.DisplaySize, IM_COL32(10, 10, 12, 255));

      float top_y = cy - 140.0f;
      if (m_logo_texture) {
        float logo_w = std::min(io.DisplaySize.x * 0.35f, 400.0f);
        float logo_h = logo_w * m_logo_texture->get_aspect_ratio();
        top_y = cy - logo_h * 0.5f - 40.0f;

        draw->AddImage((ImTextureID)(intptr_t)m_logo_texture->get_id(),
                       ImVec2(cx - logo_w * 0.5f, top_y),
                       ImVec2(cx + logo_w * 0.5f, top_y + logo_h));
        top_y += logo_h + 24.0f;
      }

      const char* controls[] = {
        "WASD  Move",  "Ctrl  Sprint",     "Shift  Sneak",
        "Space  Jump", "Left Click  Fire", "Right Click  Grab and place crate"
      };

      for (int i = 0; i < 6; ++i) {
        ImVec2 sz = ImGui::CalcTextSize(controls[i]);
        draw->AddText(nullptr,
                      15.0f,
                      ImVec2(cx - sz.x * 0.5f, top_y + i * 22.0f),
                      IM_COL32(165, 170, 180, 220),
                      controls[i]);
      }

      const char* prompt = "Press Space or Click to Begin";
      ImVec2 p_sz = ImGui::CalcTextSize(prompt);
      draw->AddText(nullptr,
                    16.0f,
                    ImVec2(cx - p_sz.x * 0.5f, top_y + 160.0f),
                    IM_COL32(230, 215, 125, 240),
                    prompt);
      return;
    }

    glm::mat4 V = m_app_cache.get_camera().get_view_matrix();
    glm::mat4 P = m_app_cache.get_camera().get_projection_matrix();
    glm::mat4 VP = P * V;

    glm::vec3 player_pos = m_player->get_local_transform().get_position();

    std::vector<BoundingBox> colliders;
    bool needs_enemy_occlusion = false;
    for (const auto& enemy : m_enemies) {
      if (enemy && !enemy->is_dead() &&
          enemy->get_state() == EnemyState::COMBAT &&
          glm::distance(enemy->get_position(), player_pos) <= 35.0f) {
        needs_enemy_occlusion = true;
        break;
      }
    }
    if (needs_enemy_occlusion)
      collect_game_colliders(colliders);
#ifndef NDEBUG
    if (m_debug_show_hitboxes) {
      std::vector<BoundingBox> debug_boxes;
      debug_boxes.reserve(2048);

      // Regular collidable scene objects.
      collect_colliders_recursive(m_scene_ptr, debug_boxes);

      // Enemy hitboxes.
      for (const auto& enemy : m_enemies) {
        if (enemy && !enemy->is_dead())
          debug_boxes.push_back(enemy->get_hitbox());
      }

      const int edges[12][2] = { { 0, 1 }, { 0, 2 }, { 1, 3 }, { 2, 3 },
                                 { 4, 5 }, { 4, 6 }, { 5, 7 }, { 6, 7 },
                                 { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };

      auto project = [&](const glm::vec3& world, ImVec2& out) {
        const glm::vec4 clip = VP * glm::vec4(world, 1.0f);

        if (clip.w <= 0.05f)
          return false;

        const glm::vec3 ndc = glm::vec3(clip) / clip.w;

        if (ndc.x < -1.5f || ndc.x > 1.5f || ndc.y < -1.5f || ndc.y > 1.5f)
          return false;

        out = ImVec2((ndc.x * 0.5f + 0.5f) * io.DisplaySize.x,
                     (1.0f - (ndc.y * 0.5f + 0.5f)) * io.DisplaySize.y);

        return true;
      };

      ImDrawList* hitbox_draw = ImGui::GetForegroundDrawList();

      std::size_t drawn_boxes = 0;

      for (const BoundingBox& box : debug_boxes) {
        const glm::vec3 mn = box.get_min();
        const glm::vec3 mx = box.get_max();

        const glm::vec3 corners[8] = {
          { mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z }, { mn.x, mx.y, mn.z },
          { mx.x, mx.y, mn.z }, { mn.x, mn.y, mx.z }, { mx.x, mn.y, mx.z },
          { mn.x, mx.y, mx.z }, { mx.x, mx.y, mx.z }
        };

        ImVec2 points[8];
        bool valid[8];

        for (int i = 0; i < 8; ++i)
          valid[i] = project(corners[i], points[i]);

        bool visible = false;

        for (const auto& edge : edges) {
          if (valid[edge[0]] && valid[edge[1]]) {
            hitbox_draw->AddLine(points[edge[0]],
                                 points[edge[1]],
                                 IM_COL32(255, 60, 60, 230),
                                 1.5f);
            visible = true;
          }
        }

        if (visible)
          ++drawn_boxes;
      }

      char label[128];
      std::snprintf(label,
                    sizeof(label),
                    "Collision hitboxes: %zu (visible: %zu)",
                    debug_boxes.size(),
                    drawn_boxes);

      hitbox_draw->AddText(
        ImVec2(16.0f, 40.0f), IM_COL32(255, 80, 80, 255), label);
    }
#endif

    for (const auto& enemy : m_enemies) {
      if (enemy->is_dead() || enemy->get_state() != EnemyState::COMBAT)
        continue;

      glm::vec3 eye_pos = enemy->get_position() + glm::vec3(0.0f, 1.9f, 0.0f);
      glm::vec3 to_player = player_pos - eye_pos;
      float dist_to_player = glm::length(to_player);
      if (dist_to_player > 35.0f)
        continue;

      bool visible = true;
      glm::vec3 ray_dir = glm::normalize(to_player);
      for (const auto& aabb : colliders) {
        float t = 0.0f;
        glm::vec3 norm(0.0f);
        if (aabb.intersects_ray(eye_pos, ray_dir, t, norm)) {
          if (t > 0.4f && t < (dist_to_player - 0.4f)) {
            visible = false;
            break;
          }
        }
      }

      if (!visible)
        continue;

      glm::vec3 head_world =
        enemy->get_position() + glm::vec3(0.0f, 2.45f, 0.0f);
      glm::vec4 clip = VP * glm::vec4(head_world, 1.0f);
      if (clip.w > 0.1f) {
        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        if (ndc.z >= -1.0f && ndc.z <= 1.0f && ndc.x >= -1.1f &&
            ndc.x <= 1.1f && ndc.y >= -1.1f && ndc.y <= 1.1f) {
          float sx = (ndc.x * 0.5f + 0.5f) * io.DisplaySize.x;
          float sy = (1.0f - (ndc.y * 0.5f + 0.5f)) * io.DisplaySize.y;

          float bar_w = glm::clamp(90.0f / (clip.w * 0.06f), 35.0f, 90.0f);
          float bar_h = 5.0f;
          float hp_pct = enemy->get_health() / enemy->get_max_health();

          draw->AddRectFilled(ImVec2(sx - bar_w * 0.5f, sy - bar_h * 0.5f),
                              ImVec2(sx + bar_w * 0.5f, sy + bar_h * 0.5f),
                              IM_COL32(10, 10, 10, 200),
                              2.5f);
          ImU32 fill_col = (enemy->get_damage_flash() > 0.0f)
                             ? IM_COL32(255, 255, 255, 255)
                             : IM_COL32(200, 40, 40, 230);
          draw->AddRectFilled(
            ImVec2(sx - bar_w * 0.5f, sy - bar_h * 0.5f),
            ImVec2(sx - bar_w * 0.5f + bar_w * hp_pct, sy + bar_h * 0.5f),
            fill_col,
            2.5f);
        }
      }
    }

    float cross_gap = 5.0f + m_gun_recoil * 50.0f;
    float cross_len = 6.0f;
    ImU32 cross_col = IM_COL32(240, 240, 240, 200);
    draw->AddLine(ImVec2(cx - cross_gap - cross_len, cy),
                  ImVec2(cx - cross_gap, cy),
                  cross_col,
                  1.2f);
    draw->AddLine(ImVec2(cx + cross_gap, cy),
                  ImVec2(cx + cross_gap + cross_len, cy),
                  cross_col,
                  1.2f);
    draw->AddLine(ImVec2(cx, cy - cross_gap - cross_len),
                  ImVec2(cx, cy - cross_gap),
                  cross_col,
                  1.2f);
    draw->AddLine(ImVec2(cx, cy + cross_gap),
                  ImVec2(cx, cy + cross_gap + cross_len),
                  cross_col,
                  1.2f);

    if (m_hitmarker_timer > 0.0f) {
      float h_size = 7.0f;
      ImU32 hit_col = IM_COL32(
        250, 250, 250, static_cast<int>((m_hitmarker_timer / 0.22f) * 240));
      draw->AddLine(ImVec2(cx - h_size, cy - h_size),
                    ImVec2(cx - 2.0f, cy - 2.0f),
                    hit_col,
                    1.5f);
      draw->AddLine(ImVec2(cx + 2.0f, cy + 2.0f),
                    ImVec2(cx + h_size, cy + h_size),
                    hit_col,
                    1.5f);
      draw->AddLine(ImVec2(cx + h_size, cy - h_size),
                    ImVec2(cx + 2.0f, cy - 2.0f),
                    hit_col,
                    1.5f);
      draw->AddLine(ImVec2(cx - 2.0f, cy + 2.0f),
                    ImVec2(cx - h_size, cy + h_size),
                    hit_col,
                    1.5f);
    }

    if (m_damage_flash_timer > 0.0f) {
      int alpha = static_cast<int>((m_damage_flash_timer / 0.28f) * 90);
      draw->AddRectFilled(
        ImVec2(0, 0), io.DisplaySize, IM_COL32(180, 20, 20, alpha));
    }

    float hp_ratio = m_player_health / m_player_max_health;
    float bar_w = 200.0f;
    draw->AddRectFilled(ImVec2(30, io.DisplaySize.y - 45),
                        ImVec2(30 + bar_w, io.DisplaySize.y - 31),
                        IM_COL32(15, 15, 15, 210),
                        3.0f);
    draw->AddRectFilled(ImVec2(30, io.DisplaySize.y - 45),
                        ImVec2(30 + bar_w * hp_ratio, io.DisplaySize.y - 31),
                        IM_COL32(40, 160, 70, 230),
                        3.0f);

    float map_cx = io.DisplaySize.x - 90.0f;
    float map_cy = io.DisplaySize.y - 90.0f;
    float map_r = 70.0f;
    draw->AddCircleFilled(
      ImVec2(map_cx, map_cy), map_r, IM_COL32(15, 15, 20, 220), 32);
    draw->AddCircle(
      ImVec2(map_cx, map_cy), map_r, IM_COL32(100, 100, 120, 200), 32, 2.0f);

    draw->AddCircleFilled(
      ImVec2(map_cx, map_cy), 4.0f, IM_COL32(60, 220, 90, 255));

    glm::vec2 scroll_delta(m_scroll_pos.x - player_pos.x,
                           m_scroll_pos.z - player_pos.z);
    if (glm::length(scroll_delta) < 160.0f) {
      float mx = map_cx + scroll_delta.x * (map_r / 160.0f);
      float my = map_cy + scroll_delta.y * (map_r / 160.0f);
      draw->AddCircleFilled(ImVec2(mx, my), 5.0f, IM_COL32(240, 210, 60, 255));
    }

    for (const auto& enemy : m_enemies) {
      if (enemy->is_dead())
        continue;
      glm::vec3 e_pos = enemy->get_position();
      glm::vec2 e_delta(e_pos.x - player_pos.x, e_pos.z - player_pos.z);
      if (glm::length(e_delta) < 160.0f) {
        float mx = map_cx + e_delta.x * (map_r / 160.0f);
        float my = map_cy + e_delta.y * (map_r / 160.0f);
        draw->AddCircleFilled(ImVec2(mx, my), 3.5f, IM_COL32(230, 50, 50, 240));
      }
    }

    if (m_game_state == GameState::PLAYING) {
      int dist = static_cast<int>(
        glm::distance(glm::vec2(player_pos.x, player_pos.z),
                      glm::vec2(m_scroll_pos.x, m_scroll_pos.z)));
      std::string text = "Scroll distance  " + std::to_string(dist) + "m";
      ImVec2 sz = ImGui::CalcTextSize(text.c_str());

      draw->AddText(nullptr,
                    18.0f,
                    ImVec2(cx - sz.x * 0.5f + 1.0f, 25.0f),
                    IM_COL32(0, 0, 0, 180),
                    text.c_str());
      draw->AddText(nullptr,
                    18.0f,
                    ImVec2(cx - sz.x * 0.5f, 24.0f),
                    IM_COL32(240, 220, 130, 240),
                    text.c_str());

    } else if (m_game_state == GameState::VICTORY) {
      const char* text = "You found the scroll";
      const char* sub = "Press R to play again";
      float text_y = io.DisplaySize.y * 0.38f;

      ImVec2 base_sz1 = ImGui::CalcTextSize(text);
      ImVec2 base_sz2 = ImGui::CalcTextSize(sub);
      float w1 = base_sz1.x * (32.0f / 14.0f);
      float w2 = base_sz2.x * (17.0f / 14.0f);

      draw->AddText(nullptr,
                    32.0f,
                    ImVec2(cx - w1 * 0.5f + 1.0f, text_y + 1.0f),
                    IM_COL32(0, 0, 0, 200),
                    text);
      draw->AddText(nullptr,
                    32.0f,
                    ImVec2(cx - w1 * 0.5f, text_y),
                    IM_COL32(110, 230, 140, 255),
                    text);

      draw->AddText(nullptr,
                    17.0f,
                    ImVec2(cx - w2 * 0.5f + 1.0f, text_y + 45.0f),
                    IM_COL32(0, 0, 0, 180),
                    sub);
      draw->AddText(nullptr,
                    17.0f,
                    ImVec2(cx - w2 * 0.5f, text_y + 44.0f),
                    IM_COL32(220, 220, 225, 230),
                    sub);

    } else if (m_game_state == GameState::DEFEATED) {
      const char* text = "You were taken down";
      const char* sub = "Press R to retry";
      float text_y = io.DisplaySize.y * 0.38f;

      ImVec2 base_sz1 = ImGui::CalcTextSize(text);
      ImVec2 base_sz2 = ImGui::CalcTextSize(sub);
      float w1 = base_sz1.x * (32.0f / 14.0f);
      float w2 = base_sz2.x * (17.0f / 14.0f);

      draw->AddText(nullptr,
                    32.0f,
                    ImVec2(cx - w1 * 0.5f + 1.0f, text_y + 1.0f),
                    IM_COL32(0, 0, 0, 200),
                    text);
      draw->AddText(nullptr,
                    32.0f,
                    ImVec2(cx - w1 * 0.5f, text_y),
                    IM_COL32(235, 80, 80, 255),
                    text);

      draw->AddText(nullptr,
                    17.0f,
                    ImVec2(cx - w2 * 0.5f + 1.0f, text_y + 45.0f),
                    IM_COL32(0, 0, 0, 180),
                    sub);
      draw->AddText(nullptr,
                    17.0f,
                    ImVec2(cx - w2 * 0.5f, text_y + 44.0f),
                    IM_COL32(220, 220, 225, 230),
                    sub);
    }
  }

  void reset_match()
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
    m_app_cache.get_camera().look_at(player_spawn +
                                     glm::vec3(0.0f, 0.0f, -1.0f));

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

  void build_massive_city(std::mt19937& rng)
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
      from_OBJ_material_groups<Vertex_PosTexNorm, TriangleIndices>(
        "city.obj", GL_TRIANGLES);
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
          city,
          [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
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
              : std::unordered_map<std::string,
                                   UniformValue>{ { "u_color", group.diffuse },
                                                  { "u_use_texture", 0 },
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

  void spawn_tactical_enemies(std::mt19937& rng)
  {
    if (!m_city_bounds_valid)
      return;

    std::uniform_real_distribution<float> x_dist(m_city_world_min.x + 12.0f,
                                                 m_city_world_max.x - 12.0f);
    std::uniform_real_distribution<float> z_dist(m_city_world_min.z + 12.0f,
                                                 m_city_world_max.z - 12.0f);
    std::uniform_real_distribution<float> offset_dist(-24.0f, 24.0f);

    const glm::vec3 player_pos = m_player->get_local_transform().get_position();
    for (int i = 0; i < 14; ++i) {
      glm::vec3 spawn_pos(0.0f, 0.85f, 0.0f);
      bool found_spawn = false;
      for (int attempt = 0; attempt < 100; ++attempt) {
        spawn_pos.x = x_dist(rng);
        spawn_pos.z = z_dist(rng);
        if (glm::distance(glm::vec2(spawn_pos.x, spawn_pos.z),
                          glm::vec2(player_pos.x, player_pos.z)) > 45.0f) {
          found_spawn = true;
          break;
        }
      }
      if (!found_spawn)
        break;

      const float ox = offset_dist(rng);
      const float oz = offset_dist(rng);
      std::vector<glm::vec3> patrol_route = {
        spawn_pos + glm::vec3(ox, 0.0f, oz), spawn_pos - glm::vec3(ox, 0.0f, oz)
      };
      m_enemies.push_back(
        std::make_shared<Enemy>(*this, spawn_pos, patrol_route));
    }
  }

  void spawn_physics_obj_prop(const std::string& filename,
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
          packet->material->get_shader_program()->set_uniform(
            "u_preserve_color", 1);
          packet->mesh->draw();
          packet->material->get_shader_program()->set_uniform(
            "u_preserve_color", 0);
        }));

    m_scene_ptr->add_child(obj);
  }

  void setup_plane()
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

  MeshDescriptor<Vertex_PosTexNorm, TriangleIndices> get_sharp_box_descriptor()
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

};
