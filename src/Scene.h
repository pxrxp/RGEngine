#pragma once
#include "Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>
#include "components.h"

class SceneObject;
using SceneObjectPtr = std::shared_ptr<SceneObject>;

class SceneObject {
private:
  std::optional<SceneObjectPtr> m_parent = std::nullopt;
  std::vector<SceneObjectPtr> m_children;

  glm::mat4 m_model_matrix = glm::mat4(1.0f);
  bool m_dirty = true;

public:
  std::optional<std::shared_ptr<RenderPacket>> render_packet = std::nullopt;

  TransformComponent m_local_transform;
  PhysicsComponent m_physics;

  void set_dirty();

  void apply_force(const glm::vec3& force);
  void apply_force(const glm::vec3& force, const glm::vec3& point_in_world_space);

  glm::mat4 get_local_transform();
  glm::mat4 get_world_transform();

  std::shared_ptr<RenderPacket> &create_render_packet(App &app);
  void add_child(SceneObjectPtr child);

  auto begin() noexcept { return m_children.begin(); }
  auto end() noexcept { return m_children.end(); }
  auto begin() const noexcept { return m_children.begin(); }
  auto end() const noexcept { return m_children.end(); }
};
