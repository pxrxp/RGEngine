#pragma once
#include "Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>

class SceneObject;
using SceneObjectPtr = std::shared_ptr<SceneObject>;

class SceneObject {
private:
  std::optional<SceneObjectPtr> m_parent = std::nullopt;
  std::vector<SceneObjectPtr> m_children = {};

  glm::vec3 m_position = glm::vec3(0.0f);
  glm::quat m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 m_scale_factors = glm::vec3(1.0f);

  glm::mat4 m_model_matrix = glm::mat4(1.0f);
  bool m_changed = true;

public:
  std::optional<std::shared_ptr<RenderPacket>> render_packet = std::nullopt;

  void rotate(const glm::vec3 &axis, float angle);
  void translate(const glm::vec3 &offset);
  void scale(const glm::vec3 &factors);
  glm::mat4 get_local_transform();
  glm::mat4 get_world_transform();


  std::shared_ptr<RenderPacket> &create_render_packet(App &app);
  void add_child(SceneObjectPtr child);

  auto begin() noexcept { return m_children.begin(); }
  auto end() noexcept { return m_children.end(); }
  auto begin() const noexcept { return m_children.begin(); }
  auto end() const noexcept { return m_children.end(); }
};
