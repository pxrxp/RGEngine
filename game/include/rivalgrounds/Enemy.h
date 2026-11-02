#pragma once
#include <atomic>
#include <cstdint>
#include <engine/SceneObject.h>
#include <memory>
#include <vector>

enum class EnemyState
{
  PATROL,
  COMBAT,
  DEAD
};

class Scene;

struct EnemyVisualState
{
  EnemyState state = EnemyState::PATROL;
  float damage_flash = 0.0f;
};

class Enemy
{
private:
  inline static std::atomic<std::uint64_t> s_next_id{ 0 };
  const std::uint64_t m_id = s_next_id.fetch_add(1, std::memory_order_relaxed);
  std::shared_ptr<EnemyVisualState> m_visual_state =
    std::make_shared<EnemyVisualState>();
  SceneObjectStrongPtr m_root;
  std::vector<SceneObjectStrongPtr> m_body_parts;
  EnemyState m_state = EnemyState::PATROL;
  float m_health = 300.0f;
  float m_max_health = 300.0f;
  float m_shoot_cooldown = 0.0f;
  float m_alert_timer = 0.0f;
  float m_regen_cooldown = 0.0f;
  float m_damage_flash = 0.0f;

  glm::vec3 m_spawn_point;
  glm::vec3 m_target_pos;
  std::vector<glm::vec3> m_waypoints;
  size_t m_current_waypoint = 0;

  static constexpr float ENEMY_COLLISION_RADIUS = 0.48f;
  static constexpr float ENEMY_COLLISION_HEIGHT = 2.2f;
  static constexpr float ENEMY_KNOCKBACK_SCALE = 0.5f;
  static constexpr float MAX_VISION_RANGE = 64.0f;
  static constexpr float HEARING_DISTANCE = 64.0f;
  static constexpr float MAX_VERTICAL_AIM = 0.65f;
  static constexpr float FOV_COSINE = 0.35f;

  bool has_line_of_sight(const Scene& scene,
                         const glm::vec3& player_pos,
                         const std::vector<BoundingBox>& colliders) const;

public:
  Enemy(Scene& scene,
        const glm::vec3& spawn_pos,
        const std::vector<glm::vec3>& waypoints);

  void update(Scene& scene,
              const glm::vec3& player_pos,
              const glm::vec3& player_cam_fwd,
              const std::vector<BoundingBox>& colliders,
              float dt,
              float& out_damage,
              bool& out_fired_shot,
              glm::vec3& out_gun_pos);

  bool take_damage(Scene& scene,
                   float damage,
                   const glm::vec3& impulse,
                   const std::vector<BoundingBox>& colliders);

  EnemyState get_state() const { return m_state; }
  float get_health() const { return m_health; }
  float get_max_health() const { return m_max_health; }
  bool is_dead() const { return m_state == EnemyState::DEAD; }
  float get_damage_flash() const { return m_damage_flash; }
  glm::vec3 get_position() const;
  SceneObjectStrongPtr get_root() const { return m_root; }
  BoundingBox get_hitbox() const;
};
