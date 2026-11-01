#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class TransformComponent
{
public:
  bool is_dirty() const;
  TransformComponent& set_dirty();

  TransformComponent& set_position(const glm::vec3& position);
  TransformComponent& set_rotation(const glm::quat& orientation);
  TransformComponent& set_scale(const glm::vec3& scale);

  TransformComponent& translate(const glm::vec3& offset);
  TransformComponent& rotate(const glm::vec3& axis, float angle_rad);
  TransformComponent& rotate_deg(const glm::vec3& axis, float angle_deg);
  TransformComponent& scale(const glm::vec3& factors);

  const glm::vec3& get_position() const;
  const glm::quat& get_rotation() const;
  const glm::vec3& get_scale() const;
  const glm::mat4& get_model_matrix() const;

private:
  glm::vec3 m_position = {0.f, 0.f, 0.f};
  glm::quat m_orientation = {1.f, 0.f, 0.f, 0.f};
  glm::vec3 m_scale = {1.f, 1.f, 1.f};

  mutable bool m_dirty = true;
  mutable glm::mat4 m_model_matrix = glm::mat4(1.f);
};
