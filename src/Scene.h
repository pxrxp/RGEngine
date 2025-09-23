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
  BoundingBox m_world_AABB;

  glm::mat4 m_model_matrix = glm::mat4(1.0f);
  bool m_dirty = true;

public:
  std::optional<std::shared_ptr<RenderPacket>> render_packet = std::nullopt;

  TransformComponent local_transform;
  PhysicsComponent physics;

  glm::mat4 get_local_transform();
  glm::mat4 get_world_transform();
  BoundingBox& get_world_AABB();

  std::shared_ptr<RenderPacket> &create_render_packet(App &app);
  void add_child(SceneObjectPtr child);

  auto begin() noexcept { return m_children.begin(); }
  auto end() noexcept { return m_children.end(); }
  auto begin() const noexcept { return m_children.begin(); }
  auto end() const noexcept { return m_children.end(); }
};
