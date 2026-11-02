#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <engine/App.h>
#include <engine/CollisionWorld.h>
#include <engine/Scene.h>
#include <engine/SceneObject.h>
#include <engine/Texture.h>
#include <engine/components/vertex_formats.h>
#include <rivalgrounds/Enemy.h>

#include <glm/glm.hpp>

#include <memory>
#include <random>
#include <string>
#include <vector>

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
  bool m_awaiting_landing = false;
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
  bool m_held_object_hidden = false;
  bool m_hide_key_was_pressed = false;

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

  void init() override;
  bool has_open_ceiling(const glm::vec3& pos);
  bool blocks_vertical_capsule(float x,
                               float z,
                               float from_x,
                               float from_z,
                               float feet_y,
                               float head_y,
                               float radius) const override;
  bool find_ground_surface_for_actor(float x,
                                     float z,
                                     float max_y,
                                     float min_y,
                                     float& out_y) const override;
  bool find_ground_surface(float x,
                           float z,
                           float max_y,
                           float min_y,
                           float& out_y) const override;
  bool find_landing_surface(float x,
                            float z,
                            float radius,
                            float previous_bottom,
                            float current_bottom,
                            float& out_y) const override;
  void setup_weapon_viewmodel();
  void update_physics() override;
  void submit_to_renderer();
  bool static_mesh_horizontal_blocked(float x,
                                      float z,
                                      float from_x,
                                      float from_z,
                                      float feet_y,
                                      float head_y) const;
  bool static_mesh_ray_hit(const glm::vec3& origin,
                           const glm::vec3& direction,
                           float max_distance,
                           float& out_distance) const override;

private:
  void collect_game_colliders(std::vector<BoundingBox>& colliders,
                              bool include_nearby_enemies = true);
  void update_character_controller(float dt);
  void handle_object_grab();
  void handle_shooting();
  void render_game_hud();
  void reset_match();
  void build_massive_city(std::mt19937& rng);
  void spawn_tactical_enemies(std::mt19937& rng);
  void spawn_physics_obj_prop(const std::string& filename,
                              const glm::vec3& pos,
                              const glm::vec3& scale,
                              const glm::vec3& color);
  void setup_plane();
  MeshDescriptor<Vertex_PosTexNorm, TriangleIndices> get_sharp_box_descriptor();
};
