#pragma once
#include "App.h"
#include "CollisionWorld.h"
#include "Enemy.h"
#include "Material.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "components/vertex_formats.h"
#include "repos/MaterialRepo.h"
#include "repos/MeshRepo.h"
#include "util/obj_model.h"
#include <imgui.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <limits>
#include <random>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

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
  static constexpr float MAX_STEP_HEIGHT = 0.65f;
  static constexpr float HOUSE_SCALE = 2.6f;
  static constexpr float HOUSE_VOXEL_WORLD_SIZE = 0.50f;
  static constexpr float HOUSE_SPATIAL_BUCKET_SIZE = 2.0f;

  const glm::vec3 m_player_spawn = { -11.58f, PLAYER_EYE_HEIGHT, -0.48f };
  glm::vec3 m_scroll_pos = { 0.0f, 0.8f, -100.0f };

  SceneObjectStrongPtr m_weapon_viewmodel;
  float m_gun_recoil = 0.0f;
  float m_muzzle_flash_timer = 0.0f;
  bool m_mouse_was_pressed = false;
  bool m_right_mouse_was_pressed = false;
  float m_damage_flash_timer = 0.0f;
  float m_hitmarker_timer = 0.0f;
  float m_incoming_tracer_timer = 0.0f;
  glm::vec3 m_last_incoming_shot_origin = { 0.0f, 0.0f, 0.0f };
  std::vector<BulletTracer> m_tracers;

  SceneObjectStrongPtr m_held_object = nullptr;
  float m_held_object_hide_cooldown = 2.0f;

  SceneObjectStrongPtr m_scroll_objective;
  std::vector<std::shared_ptr<Enemy>> m_enemies;
  SceneObjectStrongPtr m_city_object;
  CollisionWorld m_collision_world;

  glm::vec3 m_city_world_min{ 0.0f };
  glm::vec3 m_city_world_max{ 0.0f };
  bool m_city_bounds_valid = false;

  struct HouseVoxelCell
  {
    BoundingBox bounds;
    bool walkable_top = false;
  };

  struct HouseVoxelSet
  {
    std::string filename;
    glm::vec3 center{ 0.0f };
    glm::vec3 world_min{ 0.0f };
    glm::vec3 world_max{ 0.0f };
    std::vector<HouseVoxelCell> world_cells;
    bool voxelization_attempted = false;

    std::unordered_map<std::uint64_t, std::vector<std::size_t>> spatial_buckets;
  };
  std::vector<HouseVoxelSet> m_house_voxel_sets;
  std::unordered_map<std::string, std::vector<HouseVoxelCell>>
    m_house_voxel_cache;
  std::unordered_map<std::string, std::future<std::vector<HouseVoxelCell>>>
    m_house_voxel_jobs;
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
    debug_camera();

    m_player = std::make_shared<SceneObject>();
    m_player->get_physics_component().disable();
    m_player->set_collidable(false);
    m_scene_ptr->add_child(m_player);
    m_app_cache.get_camera().setup(m_player);

    setup_weapon_viewmodel();

    m_logo_texture = m_app_cache.texture_repo.load_texture(
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
    collect_game_colliders(colliders, 20.0f, false);
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
    if (m_collision_world.blocks_vertical_capsule(
          x, z, from_x, from_z, feet_y, head_y, radius))
      return true;

    const float min_x = std::min(x, from_x) - radius;
    const float max_x = std::max(x, from_x) + radius;
    const float min_z = std::min(z, from_z) - radius;
    const float max_z = std::max(z, from_z) + radius;
    const glm::ivec3 bucket_min =
      spatial_bucket_coord({ min_x, feet_y, min_z });
    const glm::ivec3 bucket_max =
      spatial_bucket_coord({ max_x, head_y, max_z });

    for (const auto& set : m_house_voxel_sets) {
      if (set.spatial_buckets.empty())
        continue;
      std::unordered_set<std::size_t> visited;
      for (int by = bucket_min.y; by <= bucket_max.y; ++by) {
        for (int bz = bucket_min.z; bz <= bucket_max.z; ++bz) {
          for (int bx = bucket_min.x; bx <= bucket_max.x; ++bx) {
            const auto bucket =
              set.spatial_buckets.find(spatial_bucket_key({ bx, by, bz }));
            if (bucket == set.spatial_buckets.end())
              continue;
            for (const std::size_t index : bucket->second) {
              if (!visited.insert(index).second ||
                  index >= set.world_cells.size())
                continue;
              const BoundingBox& bounds = set.world_cells[index].bounds;
              const glm::vec3 cell_min = bounds.get_min();
              const glm::vec3 cell_max = bounds.get_max();
              if (cell_max.y < feet_y || cell_min.y > head_y)
                continue;
              const float segment_x = x - from_x;
              const float segment_z = z - from_z;
              const float segment_length_sq =
                segment_x * segment_x + segment_z * segment_z;
              const float projection =
                segment_length_sq > 1e-8f
                  ? std::clamp(
                      ((cell_min.x + cell_max.x) * 0.5f - from_x) * segment_x +
                        ((cell_min.z + cell_max.z) * 0.5f - from_z) * segment_z,
                      0.0f,
                      segment_length_sq) /
                      segment_length_sq
                  : 0.0f;
              const float closest_x = from_x + segment_x * projection;
              const float closest_z = from_z + segment_z * projection;
              const float dx = std::max(
                { cell_min.x - closest_x, 0.0f, closest_x - cell_max.x });
              const float dz = std::max(
                { cell_min.z - closest_z, 0.0f, closest_z - cell_max.z });
              if (dx * dx + dz * dz <= radius * radius)
                return true;
            }
          }
        }
      }
    }
    return false;
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
    if (m_game_state != GameState::PLAYING) {
      if (m_app_cache.get_window().is_key_pressed(GLFW_KEY_R)) {
        reset_match();
        return;
      }
      if (m_game_state == GameState::INTRO) {
        auto& win = m_app_cache.get_window();
        if (win.is_key_pressed(GLFW_KEY_SPACE) ||
            win.is_key_pressed(GLFW_KEY_ENTER) ||
            win.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
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

    if (m_muzzle_flash_timer > 0.0f)
      m_muzzle_flash_timer -= dt;

    if (m_damage_flash_timer > 0.0f)
      m_damage_flash_timer -= dt;

    if (m_hitmarker_timer > 0.0f)
      m_hitmarker_timer -= dt;

    if (m_incoming_tracer_timer > 0.0f)
      m_incoming_tracer_timer -= dt;

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
    collect_game_colliders(colliders, 6.0f);

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
        m_incoming_tracer_timer = 0.15f;
        m_last_incoming_shot_origin = gun_pos;
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

  bool get_static_ground_height(float x,
                                float z,
                                float max_y,
                                float min_y,
                                float& out_y) const
  {
    return m_collision_world.find_ground_height(x, z, max_y, min_y, out_y);
  }

private:
  static bool triangle_intersects_box(const std::array<glm::vec3, 3>& tri,
                                      const glm::vec3& box_min,
                                      const glm::vec3& box_max)
  {
    const glm::vec3 center = (box_min + box_max) * 0.5f;
    const glm::vec3 half = (box_max - box_min) * 0.5f;
    const glm::vec3 v0 = tri[0] - center;
    const glm::vec3 v1 = tri[1] - center;
    const glm::vec3 v2 = tri[2] - center;
    const glm::vec3 edges[3] = { v1 - v0, v2 - v1, v0 - v2 };

    auto separates = [&](const glm::vec3& axis) {
      if (glm::dot(axis, axis) < 1e-12f)
        return false;
      const float p0 = glm::dot(v0, axis);
      const float p1 = glm::dot(v1, axis);
      const float p2 = glm::dot(v2, axis);
      const float tri_min = std::min(p0, std::min(p1, p2));
      const float tri_max = std::max(p0, std::max(p1, p2));
      const float radius = half.x * std::abs(axis.x) +
                           half.y * std::abs(axis.y) +
                           half.z * std::abs(axis.z);
      return tri_min > radius || tri_max < -radius;
    };

    if (std::max(v0.x, std::max(v1.x, v2.x)) < -half.x ||
        std::min(v0.x, std::min(v1.x, v2.x)) > half.x ||
        std::max(v0.y, std::max(v1.y, v2.y)) < -half.y ||
        std::min(v0.y, std::min(v1.y, v2.y)) > half.y ||
        std::max(v0.z, std::max(v1.z, v2.z)) < -half.z ||
        std::min(v0.z, std::min(v1.z, v2.z)) > half.z)
      return false;

    const glm::vec3 box_axes[3] = { glm::vec3(1, 0, 0),
                                    glm::vec3(0, 1, 0),
                                    glm::vec3(0, 0, 1) };
    for (const auto& edge : edges) {
      for (const auto& axis : box_axes) {
        if (separates(glm::cross(edge, axis)))
          return false;
      }
    }

    return !separates(glm::cross(edges[0], edges[1]));
  }

  static bool ray_intersects_bounds(const glm::vec3& origin,
                                    const glm::vec3& direction,
                                    const glm::vec3& bounds_min,
                                    const glm::vec3& bounds_max,
                                    float max_distance)
  {
    float t_min = 0.0f;
    float t_max = max_distance;
    for (int axis = 0; axis < 3; ++axis) {
      const float o = origin[axis];
      const float d = direction[axis];
      if (std::abs(d) < 1e-6f) {
        if (o < bounds_min[axis] || o > bounds_max[axis])
          return false;
        continue;
      }
      float t1 = (bounds_min[axis] - o) / d;
      float t2 = (bounds_max[axis] - o) / d;
      if (t1 > t2)
        std::swap(t1, t2);
      t_min = std::max(t_min, t1);
      t_max = std::min(t_max, t2);
      if (t_min > t_max)
        return false;
    }
    return t_max >= 0.0f;
  }

  static std::uint64_t spatial_bucket_key(const glm::ivec3& cell)
  {

    constexpr std::uint64_t MASK = (1ull << 21) - 1ull;
    return ((static_cast<std::uint64_t>(cell.x) & MASK) << 42) |
           ((static_cast<std::uint64_t>(cell.y) & MASK) << 21) |
           (static_cast<std::uint64_t>(cell.z) & MASK);
  }

  static glm::ivec3 spatial_bucket_coord(const glm::vec3& p)
  {
    return glm::ivec3(
      static_cast<int>(std::floor(p.x / HOUSE_SPATIAL_BUCKET_SIZE)),
      static_cast<int>(std::floor(p.y / HOUSE_SPATIAL_BUCKET_SIZE)),
      static_cast<int>(std::floor(p.z / HOUSE_SPATIAL_BUCKET_SIZE)));
  }

  struct CollisionTriangle
  {
    std::array<glm::vec3, 3> p;
    glm::vec3 min;
    glm::vec3 max;
  };

  struct CollisionTriangleMesh
  {
    std::vector<CollisionTriangle> triangles;
    glm::vec3 min{ 0.0f };
    glm::vec3 max{ 0.0f };
  };

  static void finalize_collision_mesh(CollisionTriangleMesh& mesh)
  {
    if (mesh.triangles.empty())
      return;
    mesh.min = glm::vec3(std::numeric_limits<float>::max());
    mesh.max = glm::vec3(std::numeric_limits<float>::lowest());
    for (auto& tri : mesh.triangles) {
      tri.min = glm::min(tri.p[0], glm::min(tri.p[1], tri.p[2]));
      tri.max = glm::max(tri.p[0], glm::max(tri.p[1], tri.p[2]));
      mesh.min = glm::min(mesh.min, tri.min);
      mesh.max = glm::max(mesh.max, tri.max);
    }
  }

  static bool load_original_house_collision_mesh(
    const std::string& extracted_filename,
    CollisionTriangleMesh& out)
  {
    std::ifstream extracted("resources/" + extracted_filename);
    if (!extracted)
      return false;
    std::string header;
    std::getline(extracted, header);
    if (header.rfind("# ", 0) != 0)
      return false;
    const auto delim = header.find(" - ");
    if (delim == std::string::npos)
      return false;
    const std::string wanted_group = header.substr(2, delim - 2);
    if (wanted_group.empty())
      return false;

    std::ifstream source("resources/Houses.obj");
    if (!source)
      source.open("Houses.obj");
    if (!source)
      source.open("/tmp/mozilla_admin0/Houses.obj");
    if (!source) {
      std::fprintf(stderr,
                   "Original Houses.obj not found (tried resources/Houses.obj, "
                   "./Houses.obj, and /tmp/mozilla_admin0/Houses.obj); "
                   "collision will fall back to the extracted house mesh.\n");
      return false;
    }

    std::vector<glm::vec3> positions;
    std::string current_group = "default";
    std::string line;
    while (std::getline(source, line)) {
      std::istringstream row(line);
      std::string tag;
      row >> tag;
      if (tag.empty() || tag[0] == '#')
        continue;
      if (tag == "v") {
        float x, y, z;
        if (row >> x >> y >> z) {

          positions.emplace_back(x * 0.01f, z * 0.01f, -y * 0.01f);
        }
      } else if (tag == "g" || tag == "o") {
        row >> current_group;
      } else if (tag == "f" && current_group == wanted_group) {
        std::vector<int> face;
        std::string corner;
        while (row >> corner) {
          const auto slash = corner.find('/');
          const std::string index_text = corner.substr(0, slash);
          if (index_text.empty())
            continue;
          int index = 0;
          try {
            index = std::stoi(index_text);
          } catch (...) {
            index = 0;
          }
          const int resolved =
            index > 0 ? index - 1 : static_cast<int>(positions.size()) + index;
          if (resolved >= 0 &&
              static_cast<std::size_t>(resolved) < positions.size())
            face.push_back(resolved);
        }
        if (face.size() >= 3) {
          for (std::size_t i = 1; i + 1 < face.size(); ++i) {
            CollisionTriangle tri;
            tri.p = { positions[face[0]],
                      positions[face[i]],
                      positions[face[i + 1]] };
            out.triangles.push_back(tri);
          }
        }
      }
    }
    if (out.triangles.empty())
      return false;

    finalize_collision_mesh(out);
    const glm::vec3 offset((out.min.x + out.max.x) * 0.5f,
                           out.min.y,
                           (out.min.z + out.max.z) * 0.5f);
    for (auto& tri : out.triangles)
      for (auto& point : tri.p)
        point -= offset;
    finalize_collision_mesh(out);
    return true;
  }

  static CollisionTriangleMesh collision_mesh_from_descriptor(
    const MeshDescriptor<Vertex_PosTexNorm, TriangleIndices>& desc)
  {
    CollisionTriangleMesh mesh;
    if (desc.vertices.empty() || desc.indices.empty())
      return mesh;
    mesh.triangles.reserve(desc.indices.size());
    for (const auto& idx : desc.indices) {
      if (idx.vert1 >= desc.vertices.size() ||
          idx.vert2 >= desc.vertices.size() ||
          idx.vert3 >= desc.vertices.size())
        continue;
      CollisionTriangle tri;
      tri.p = { desc.vertices[idx.vert1].position,
                desc.vertices[idx.vert2].position,
                desc.vertices[idx.vert3].position };
      mesh.triangles.push_back(tri);
    }
    finalize_collision_mesh(mesh);
    return mesh;
  }

  static std::vector<HouseVoxelCell> build_house_voxels_from_mesh(
    const CollisionTriangleMesh& mesh)
  {

    constexpr float CELL_SIZE = HOUSE_VOXEL_WORLD_SIZE / HOUSE_SCALE;
    constexpr std::size_t MAX_TRIANGLE_CELL_TESTS = 12'000'000;
    constexpr std::size_t MAX_RAW_CELLS = 150'000;
    constexpr std::size_t MAX_OUTPUT_CELLS = 32'000;
    constexpr int MAX_GRID_AXIS = 2048;
    if (mesh.triangles.empty())
      return {};

    const glm::vec3 extent = mesh.max - mesh.min;
    if (!std::isfinite(extent.x) || !std::isfinite(extent.y) ||
        !std::isfinite(extent.z) || extent.x <= 1e-5f || extent.y <= 1e-5f ||
        extent.z <= 1e-5f)
      return {};

    const int nx =
      std::max(1, static_cast<int>(std::ceil(extent.x / CELL_SIZE)));
    const int ny =
      std::max(1, static_cast<int>(std::ceil(extent.y / CELL_SIZE)));
    const int nz =
      std::max(1, static_cast<int>(std::ceil(extent.z / CELL_SIZE)));
    if (nx > MAX_GRID_AXIS || ny > MAX_GRID_AXIS || nz > MAX_GRID_AXIS) {
      std::fprintf(stderr,
                   "House voxel grid axis exceeds safety limit (%d, %d, %d).\n",
                   nx,
                   ny,
                   nz);
      return {};
    }

    auto key_for = [](int x, int y, int z) -> std::uint64_t {
      return ((static_cast<std::uint64_t>(x) & ((1ull << 21) - 1ull)) << 42) |
             ((static_cast<std::uint64_t>(y) & ((1ull << 21) - 1ull)) << 21) |
             (static_cast<std::uint64_t>(z) & ((1ull << 21) - 1ull));
    };
    auto decode_key = [](std::uint64_t key) -> glm::ivec3 {
      constexpr std::uint64_t mask = (1ull << 21) - 1ull;
      return glm::ivec3(static_cast<int>((key >> 42) & mask),
                        static_cast<int>((key >> 21) & mask),
                        static_cast<int>(key & mask));
    };

    std::unordered_map<std::uint64_t, bool> occupied;
    occupied.reserve(
      std::min<std::size_t>(MAX_RAW_CELLS, mesh.triangles.size() * 8));
    std::size_t triangle_cell_tests = 0;
    bool aborted = false;

    for (const auto& tri : mesh.triangles) {
      int x0 = std::clamp(
        static_cast<int>(std::floor((tri.min.x - mesh.min.x) / CELL_SIZE)),
        0,
        nx - 1);
      int y0 = std::clamp(
        static_cast<int>(std::floor((tri.min.y - mesh.min.y) / CELL_SIZE)),
        0,
        ny - 1);
      int z0 = std::clamp(
        static_cast<int>(std::floor((tri.min.z - mesh.min.z) / CELL_SIZE)),
        0,
        nz - 1);
      int x1 = std::clamp(
        static_cast<int>(std::floor((tri.max.x - mesh.min.x) / CELL_SIZE)),
        0,
        nx - 1);
      int y1 = std::clamp(
        static_cast<int>(std::floor((tri.max.y - mesh.min.y) / CELL_SIZE)),
        0,
        ny - 1);
      int z1 = std::clamp(
        static_cast<int>(std::floor((tri.max.z - mesh.min.z) / CELL_SIZE)),
        0,
        nz - 1);

      const glm::vec3 normal =
        glm::cross(tri.p[1] - tri.p[0], tri.p[2] - tri.p[0]);
      const float normal_len = glm::length(normal);
      const bool horizontal_surface =
        normal_len > 1e-7f && std::abs(normal.y) / normal_len >= 0.70f;

      for (int x = x0; x <= x1 && !aborted; ++x) {
        for (int y = y0; y <= y1 && !aborted; ++y) {
          for (int z = z0; z <= z1; ++z) {
            if (++triangle_cell_tests > MAX_TRIANGLE_CELL_TESTS) {
              aborted = true;
              break;
            }
            const glm::vec3 cell_min =
              mesh.min + glm::vec3(x, y, z) * CELL_SIZE;
            const glm::vec3 cell_max =
              glm::min(cell_min + glm::vec3(CELL_SIZE), mesh.max);
            if (!triangle_intersects_box(tri.p, cell_min, cell_max))
              continue;

            const std::uint64_t key = key_for(x, y, z);
            auto [it, inserted] = occupied.emplace(key, horizontal_surface);
            if (!inserted && horizontal_surface)
              it->second = true;
            if (occupied.size() > MAX_RAW_CELLS) {
              aborted = true;
              break;
            }
          }
        }
      }
      if (aborted)
        break;
    }

    if (aborted) {
      std::fprintf(
        stderr,
        "House voxelization hit safety budget (triangle-cell tests=%zu, raw "
        "cells=%zu); model collision was discarded.\n",
        triangle_cell_tests,
        occupied.size());
      return {};
    }
    if (occupied.empty())
      return {};

    struct MergeNode
    {
      int x, y, z;
      int size;
      bool full;
      bool walkable_top;
    };
    std::vector<MergeNode> active;
    active.reserve(occupied.size());
    for (const auto& [key, walkable] : occupied) {
      const glm::ivec3 c = decode_key(key);
      const glm::vec3 cell_min = mesh.min + glm::vec3(c) * CELL_SIZE;
      const glm::vec3 cell_max =
        glm::min(cell_min + glm::vec3(CELL_SIZE), mesh.max);
      const glm::vec3 actual_size = cell_max - cell_min;
      const bool full = std::abs(actual_size.x - CELL_SIZE) < 1e-5f &&
                        std::abs(actual_size.y - CELL_SIZE) < 1e-5f &&
                        std::abs(actual_size.z - CELL_SIZE) < 1e-5f;
      active.push_back({ c.x, c.y, c.z, 1, full, walkable });
    }

    std::vector<MergeNode> final_nodes;
    final_nodes.reserve(active.size());
    int level_size = 1;
    while (!active.empty()) {
      std::unordered_map<std::uint64_t, std::size_t> lookup;
      lookup.reserve(active.size());
      for (std::size_t i = 0; i < active.size(); ++i)
        lookup.emplace(key_for(active[i].x, active[i].y, active[i].z), i);
      std::vector<bool> consumed(active.size(), false);
      std::vector<MergeNode> parents;
      parents.reserve(active.size() / 8);

      for (std::size_t i = 0; i < active.size(); ++i) {
        const auto& node = active[i];
        if (consumed[i] || !node.full || node.x % (level_size * 2) != 0 ||
            node.y % (level_size * 2) != 0 || node.z % (level_size * 2) != 0)
          continue;

        std::array<std::size_t, 8> siblings{};
        bool can_merge = true;
        bool parent_walkable_top = false;
        for (int child = 0; child < 8; ++child) {
          const int cx = node.x + ((child & 1) ? level_size : 0);
          const int cy = node.y + ((child & 2) ? level_size : 0);
          const int cz = node.z + ((child & 4) ? level_size : 0);
          auto found = lookup.find(key_for(cx, cy, cz));
          if (found == lookup.end() || !active[found->second].full ||
              consumed[found->second]) {
            can_merge = false;
            break;
          }
          siblings[child] = found->second;
          if ((child & 2) && active[found->second].walkable_top)
            parent_walkable_top = true;
        }
        if (!can_merge)
          continue;

        for (const std::size_t sibling : siblings)
          consumed[sibling] = true;
        parents.push_back({ node.x,
                            node.y,
                            node.z,
                            level_size * 2,
                            true,
                            parent_walkable_top });
      }

      for (std::size_t i = 0; i < active.size(); ++i) {
        if (!consumed[i])
          final_nodes.push_back(active[i]);
      }
      if (parents.empty())
        break;
      active = std::move(parents);
      level_size *= 2;
    }

    if (final_nodes.size() > MAX_OUTPUT_CELLS) {
      std::fprintf(stderr,
                   "House voxel output exceeds limit (%zu cells); model "
                   "collision was discarded.\n",
                   final_nodes.size());
      return {};
    }

    std::vector<HouseVoxelCell> cells;
    cells.reserve(final_nodes.size());
    for (const auto& node : final_nodes) {
      const glm::vec3 mn =
        mesh.min + glm::vec3(node.x, node.y, node.z) * CELL_SIZE;
      const glm::vec3 mx =
        glm::min(mn + glm::vec3(node.size * CELL_SIZE), mesh.max);
      if (mn.x >= mx.x || mn.y >= mx.y || mn.z >= mx.z)
        continue;
      HouseVoxelCell cell;
      cell.bounds.add_point(mn);
      cell.bounds.add_point(mx);
      cell.walkable_top = node.walkable_top;
      cells.push_back(std::move(cell));
    }
    return cells;
  }

  static std::vector<HouseVoxelCell> build_house_voxels(
    const MeshDescriptor<Vertex_PosTexNorm, TriangleIndices>& desc)
  {
    return build_house_voxels_from_mesh(collision_mesh_from_descriptor(desc));
  }

  static std::vector<HouseVoxelCell> build_house_voxels_for_model(
    const std::string& filename)
  {
    CollisionTriangleMesh original_mesh;
    if (load_original_house_collision_mesh(filename, original_mesh)) {
      auto cells = build_house_voxels_from_mesh(original_mesh);
      if (cells.empty())
        std::fprintf(stderr,
                     "WARNING: no collision voxels generated from Houses.obj "
                     "group for %s.\n",
                     filename.c_str());
      return cells;
    }

    auto desc =
      from_OBJ<Vertex_PosTexNorm, TriangleIndices>(filename, GL_TRIANGLES);
    auto cells = build_house_voxels(desc);
    if (cells.empty())
      std::fprintf(
        stderr,
        "WARNING: no collision voxels generated from extracted model %s.\n",
        filename.c_str());
    return cells;
  }

  void collect_game_colliders(
    std::vector<BoundingBox>& colliders,
    float house_query_radius = 6.0f,
    bool include_nearby_enemies = true,
    std::vector<BoundingBox>* walkable_house_tops = nullptr,
    std::size_t* scene_collider_count = nullptr)
  {
    colliders.clear();
    if (walkable_house_tops)
      walkable_house_tops->clear();
    collect_colliders_recursive(m_scene_ptr, colliders);
    if (scene_collider_count)
      *scene_collider_count = colliders.size();

    std::vector<glm::vec3> query_centers;
    glm::vec3 player_center(0.0f);
    const bool has_player = static_cast<bool>(m_player);
    if (has_player) {
      player_center = m_player->get_local_transform().get_position();
      query_centers.push_back(player_center);
    }
    if (include_nearby_enemies) {
      for (const auto& enemy : m_enemies) {
        if (!enemy || enemy->is_dead())
          continue;
        const BoundingBox hitbox = enemy->get_hitbox();
        query_centers.push_back((hitbox.get_min() + hitbox.get_max()) * 0.5f);
      }
    }

    constexpr float MODEL_CACHE_UNLOAD_RADIUS = 42.0f;
    constexpr float INSTANCE_UNLOAD_RADIUS = 20.0f;
    constexpr float INSTANCE_BUILD_RADIUS = 16.0f;
    constexpr float VOXEL_PRELOAD_RADIUS = 34.0f;
    const float model_unload_radius_sq =
      MODEL_CACHE_UNLOAD_RADIUS * MODEL_CACHE_UNLOAD_RADIUS;
    const float instance_unload_radius_sq =
      INSTANCE_UNLOAD_RADIUS * INSTANCE_UNLOAD_RADIUS;
    const float instance_build_radius_sq =
      INSTANCE_BUILD_RADIUS * INSTANCE_BUILD_RADIUS;
    const float preload_radius_sq = VOXEL_PRELOAD_RADIUS * VOXEL_PRELOAD_RADIUS;

    for (auto& set : m_house_voxel_sets) {
      bool keep_instance = false;
      for (const auto& center : query_centers) {
        const glm::vec3 closest =
          glm::clamp(center, set.world_min, set.world_max);
        if (glm::distance2(center, closest) <= instance_unload_radius_sq) {
          keep_instance = true;
          break;
        }
      }
      if (!keep_instance) {
        std::vector<HouseVoxelCell>().swap(set.world_cells);
        decltype(set.spatial_buckets)().swap(set.spatial_buckets);
        set.voxelization_attempted = false;
      }
    }

    if (query_centers.empty()) {
      for (auto& set : m_house_voxel_sets) {
        std::vector<HouseVoxelCell>().swap(set.world_cells);
        decltype(set.spatial_buckets)().swap(set.spatial_buckets);
        set.voxelization_attempted = false;
      }
      m_house_voxel_cache.clear();
      return;
    }

    const float radius_sq = house_query_radius * house_query_radius;

    if (m_house_voxel_jobs.empty()) {
      std::string nearest_missing_model;
      float nearest_missing_distance_sq =
        std::numeric_limits<float>::infinity();
      for (const auto& set : m_house_voxel_sets) {
        if (m_house_voxel_cache.find(set.filename) != m_house_voxel_cache.end())
          continue;
        float nearest_distance_sq = std::numeric_limits<float>::infinity();
        for (const auto& center : query_centers) {
          const glm::vec3 closest =
            glm::clamp(center, set.world_min, set.world_max);
          nearest_distance_sq =
            std::min(nearest_distance_sq, glm::distance2(center, closest));
        }
        if (nearest_distance_sq <= preload_radius_sq &&
            nearest_distance_sq < nearest_missing_distance_sq) {
          nearest_missing_distance_sq = nearest_distance_sq;
          nearest_missing_model = set.filename;
        }
      }
      if (!nearest_missing_model.empty()) {
        const std::string filename = nearest_missing_model;
        m_house_voxel_jobs.emplace(
          filename, std::async(std::launch::async, [filename]() {
            return RivalGrounds::build_house_voxels_for_model(filename);
          }));
      }
    }

    for (auto& set : m_house_voxel_sets) {
      bool set_preload = false;
      bool set_build_instance = false;
      for (const auto& center : query_centers) {
        const glm::vec3 closest =
          glm::clamp(center, set.world_min, set.world_max);
        const float distance_sq = glm::distance2(center, closest);
        if (distance_sq <= preload_radius_sq)
          set_preload = true;
        if (distance_sq <= instance_build_radius_sq)
          set_build_instance = true;
      }
      if (!set_preload)
        continue;

      auto cache_it = m_house_voxel_cache.find(set.filename);
      if (cache_it == m_house_voxel_cache.end()) {
        auto job_it = m_house_voxel_jobs.find(set.filename);
        if (job_it == m_house_voxel_jobs.end())
          continue;
        if (job_it->second.wait_for(std::chrono::milliseconds(0)) !=
            std::future_status::ready)
          continue;
        auto generated = job_it->second.get();
        m_house_voxel_jobs.erase(job_it);
        cache_it =
          m_house_voxel_cache.emplace(set.filename, std::move(generated)).first;
      }

      if (!set_build_instance)
        continue;
      if (!set.voxelization_attempted) {
        set.voxelization_attempted = true;
        set.world_cells.reserve(cache_it->second.size());
        for (const auto& local_cell : cache_it->second) {
          const glm::vec3 world_min =
            set.center + local_cell.bounds.get_min() * HOUSE_SCALE;
          const glm::vec3 world_max =
            set.center + local_cell.bounds.get_max() * HOUSE_SCALE;
          HouseVoxelCell world_cell;
          world_cell.bounds.add_point(world_min);
          world_cell.bounds.add_point(world_max);
          world_cell.walkable_top = local_cell.walkable_top;
          const std::size_t cell_index = set.world_cells.size();
          set.world_cells.push_back(std::move(world_cell));
          const glm::ivec3 bmin = spatial_bucket_coord(world_min);
          const glm::ivec3 bmax = spatial_bucket_coord(world_max);
          for (int bz = bmin.z; bz <= bmax.z; ++bz) {
            for (int by = bmin.y; by <= bmax.y; ++by) {
              for (int bx = bmin.x; bx <= bmax.x; ++bx) {
                set.spatial_buckets[spatial_bucket_key(glm::ivec3(bx, by, bz))]
                  .push_back(cell_index);
              }
            }
          }
        }
      }
      if (set.spatial_buckets.empty())
        continue;

      std::vector<std::size_t> candidates;
      for (const auto& center : query_centers) {
        const glm::vec3 qmin =
          glm::max(center - glm::vec3(house_query_radius), set.world_min);
        const glm::vec3 qmax =
          glm::min(center + glm::vec3(house_query_radius), set.world_max);
        if (qmin.x > qmax.x || qmin.y > qmax.y || qmin.z > qmax.z)
          continue;
        const glm::ivec3 bmin = spatial_bucket_coord(qmin);
        const glm::ivec3 bmax = spatial_bucket_coord(qmax);
        for (int z = bmin.z; z <= bmax.z; ++z) {
          for (int y = bmin.y; y <= bmax.y; ++y) {
            for (int x = bmin.x; x <= bmax.x; ++x) {
              auto it = set.spatial_buckets.find(
                spatial_bucket_key(glm::ivec3(x, y, z)));
              if (it != set.spatial_buckets.end())
                candidates.insert(
                  candidates.end(), it->second.begin(), it->second.end());
            }
          }
        }
      }
      std::sort(candidates.begin(), candidates.end());
      candidates.erase(std::unique(candidates.begin(), candidates.end()),
                       candidates.end());

      for (const std::size_t index : candidates) {
        if (index >= set.world_cells.size())
          continue;
        const auto& cell = set.world_cells[index];
        const glm::vec3 cell_min = cell.bounds.get_min();
        const glm::vec3 cell_max = cell.bounds.get_max();
        for (const auto& center : query_centers) {
          const glm::vec3 closest = glm::clamp(center, cell_min, cell_max);
          if (glm::distance2(center, closest) <= radius_sq) {
            colliders.push_back(cell.bounds);
            if (walkable_house_tops && cell.walkable_top)
              walkable_house_tops->push_back(cell.bounds);
            break;
          }
        }
      }
    }

    for (auto it = m_house_voxel_cache.begin();
         it != m_house_voxel_cache.end();) {
      bool model_in_use = false;
      for (const auto& set : m_house_voxel_sets) {
        if (set.filename != it->first)
          continue;
        if (set.voxelization_attempted && !set.world_cells.empty()) {
          model_in_use = true;
          break;
        }
        for (const auto& center : query_centers) {
          const glm::vec3 closest =
            glm::clamp(center, set.world_min, set.world_max);
          if (glm::distance2(center, closest) <= model_unload_radius_sq) {
            model_in_use = true;
            break;
          }
        }
        if (model_in_use)
          break;
      }
      if (model_in_use) {
        ++it;
      } else {
        it = m_house_voxel_cache.erase(it);
      }
    }

    for (auto it = m_house_voxel_jobs.begin();
         it != m_house_voxel_jobs.end();) {
      bool model_needed = false;
      for (const auto& set : m_house_voxel_sets) {
        if (set.filename != it->first)
          continue;
        for (const auto& center : query_centers) {
          const glm::vec3 closest =
            glm::clamp(center, set.world_min, set.world_max);
          if (glm::distance2(center, closest) <= model_unload_radius_sq) {
            model_needed = true;
            break;
          }
        }
        if (model_needed)
          break;
      }
      if (it->second.wait_for(std::chrono::milliseconds(0)) ==
          std::future_status::ready) {
        auto generated = it->second.get();
        if (model_needed)
          m_house_voxel_cache.emplace(it->first, std::move(generated));

        it = m_house_voxel_jobs.erase(it);
      } else {
        ++it;
      }
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
    std::vector<BoundingBox> walkable_house_tops;
    std::size_t scene_collider_count = 0;
#ifndef NDEBUG
    if (!m_debug_noclip)
#endif
      collect_game_colliders(
        colliders, 2.5f, false, &walkable_house_tops, &scene_collider_count);

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
      const std::size_t ordinary_count =
        std::min(scene_collider_count, colliders.size());
      for (std::size_t i = 0; i < ordinary_count; ++i)
        consider_landing(colliders[i]);
      for (const auto& top : walkable_house_tops)
        consider_landing(top);

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
      m_muzzle_flash_timer = 0.06f;

      std::vector<BoundingBox> colliders;
      collect_game_colliders(colliders, 18.0f, false);

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
      collect_game_colliders(colliders, 6.0f);
#ifndef NDEBUG
    if (m_debug_show_hitboxes) {
      std::vector<BoundingBox> debug_boxes;
      debug_boxes.reserve(2048);

      // Regular collidable scene objects.
      collect_colliders_recursive(m_scene_ptr, debug_boxes);

      // House collision is represented by voxel AABBs, not necessarily
      // by the scene object's render AABB.
      for (const auto& set : m_house_voxel_sets) {
        for (const auto& cell : set.world_cells)
          debug_boxes.push_back(cell.bounds);
      }

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
    m_house_voxel_sets.clear();
    m_city_object.reset();
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
      m_city_object = city;
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

      m_city_object = std::make_shared<SceneObject>();
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

  void spawn_house(const std::string& filename, const glm::vec3& pos)
  {
    auto house = std::make_shared<SceneObject>();
    house->get_local_transform().set_position(pos).set_scale(
      glm::vec3(HOUSE_SCALE));
    house->get_physics_component().disable();

    house->set_collidable(false);

    auto desc =
      from_OBJ<Vertex_PosTexNorm, TriangleIndices>(filename, GL_TRIANGLES);
    auto mesh = m_app_cache.mesh_repo.load_mesh(desc);

    glm::vec3 local_min(std::numeric_limits<float>::max());
    glm::vec3 local_max(std::numeric_limits<float>::lowest());
    for (const auto& vertex : desc.vertices) {
      local_min = glm::min(local_min, vertex.position);
      local_max = glm::max(local_max, vertex.position);
    }

    HouseVoxelSet voxel_set;
    voxel_set.filename = filename;
    voxel_set.center = pos;
    voxel_set.world_min = pos + local_min * HOUSE_SCALE;
    voxel_set.world_max = pos + local_max * HOUSE_SCALE;

    m_house_voxel_sets.push_back(std::move(voxel_set));

    house->set_render_packet(
      mesh,
      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "mat_" + filename,
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = { { .texture_name = "concrete" } },
        .uniforms = { { "u_diffuse", TextureName("concrete") },
                      { "u_use_texture", 1 },
                      { "u_is_gun", 0 } } }),
      capture_weak(
        house, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        }));

    m_scene_ptr->add_child(house);
  }

  void spawn_obj_prop(const std::string& filename,
                      const glm::vec3& pos,
                      const glm::vec3& scale,
                      const glm::vec3& color,
                      bool rotate_tent = false,
                      bool collidable = false)
  {
    auto obj = std::make_shared<SceneObject>();
    obj->get_local_transform().set_position(pos).set_scale(scale);
    if (rotate_tent) {
      obj->get_local_transform().rotate_deg(AXIS_X, -90.0f);
    }
    obj->get_physics_component().disable();
    obj->set_collidable(collidable);

    obj->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        from_OBJ<Vertex_PosTexNorm, TriangleIndices>(filename, GL_TRIANGLES)),
      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "prop_mat_" + filename + "_" + std::to_string(pos.x),
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = {},
        .uniforms = { { "u_color", color },
                      { "u_use_texture", 0 },
                      { "u_is_gun", 0 } } }),
      capture_weak(obj,
                   [this]([[maybe_unused]] SceneObjectStrongPtr self,
                          RenderPacketStrongPtr packet) {
                     set_view_matrix(packet);
                     set_projection_matrix(packet);
                     set_model_matrix(packet,
                                      self->get_world_transformation_mat());
                     packet->mesh->draw();
                   }));

    m_scene_ptr->add_child(obj);
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
    obj->get_physics_component()
      .set_mass(2.5f)
      .set_drag(0.55f)
      .set_restitution(0.08f)
      .set_gravity(true);
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

  void add_box(const glm::vec3& center,
               const glm::vec3& size,
               const glm::vec3& color)
  {
    auto obj = std::make_shared<SceneObject>();
    obj->get_local_transform().set_position(center).set_scale(size);
    obj->get_physics_component().disable();
    obj->set_collidable(true);

    std::string mat_name = "mat_" + std::to_string(center.x) + "_" +
                           std::to_string(center.z) + "_" +
                           std::to_string(size.y);

    obj->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(get_sharp_box_descriptor()),
      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = mat_name,
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = { { .texture_name = "concrete" } },
        .uniforms = { { "u_diffuse", TextureName("concrete") },
                      { "u_color", color },
                      { "u_use_texture", 1 },
                      { "u_is_gun", 0 } } }),
      capture_weak(obj,
                   [this]([[maybe_unused]] SceneObjectStrongPtr self,
                          RenderPacketStrongPtr packet) {
                     set_view_matrix(packet);
                     set_projection_matrix(packet);
                     set_model_matrix(packet,
                                      self->get_world_transformation_mat());
                     packet->mesh->draw();
                   }));

    m_scene_ptr->add_child(obj);
  }
};
