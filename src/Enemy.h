#pragma once
#include "SceneObject.h"
#include <vector>

enum class EnemyState
{
  PATROL,
  INVESTIGATE,
  HUNT,
  DEAD
};

class Scene;

class Enemy
{
private:
  SceneObjectStrongPtr m_root;
  SceneObjectStrongPtr m_torso;
  SceneObjectStrongPtr m_head;
  SceneObjectStrongPtr m_visor;

  EnemyState m_state = EnemyState::PATROL;
  float m_health = 100.0f;
  float m_speed = 3.0f;
  float m_alert_timer = 0.0f;

  glm::vec3 m_target_pos = { 0.f, 0.f, 0.f };
  std::vector<glm::vec3> m_waypoints;
  size_t m_current_waypoint = 0;

  bool has_line_of_sight(const glm::vec3& player_pos,
                         const std::vector<BoundingBox>& colliders) const;

public:
  Enemy(Scene& scene,
        const glm::vec3& spawn_pos,
        const std::vector<glm::vec3>& waypoints);

  void update(Scene& scene, float dt);
  bool take_damage(float damage, const glm::vec3& impulse);

  EnemyState get_state() const { return m_state; }
  float get_health() const { return m_health; }
  bool is_dead() const { return m_state == EnemyState::DEAD; }
  glm::vec3 get_position() const;
  BoundingBox get_hitbox() const;
  SceneObjectStrongPtr get_root() const { return m_root; }
};
