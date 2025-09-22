#pragma once
#include <optional>
#include "Renderer.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class SceneObject {
private:
  std::optional<RenderPacket> m_root_object;
  std::optional<std::unique_ptr<SceneObject>> m_parent;
  std::vector<std::unique_ptr<SceneObject>> m_children;

  glm::vec3 m_position = glm::vec3(0.0f);
  glm::quat m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 m_scale_factors = glm::vec3(1.0f);

  glm::mat4 m_model_matrix = glm::mat4(1.0f);
  bool m_changed = true;

public:
  void rotate(const glm::vec3 &axis, float angle);
  void translate(const glm::vec3 &offset);
  void scale(const glm::vec3 &factors);
  glm::mat4 get_local_transform();
  glm::mat4 get_world_transform();
};
