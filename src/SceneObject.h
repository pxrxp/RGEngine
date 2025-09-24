#pragma once
#include "Renderer.h"
#include "components/PhysicsComponent.h"
#include "components/TransformComponent.h"
#include "components/BoundingBox.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>

class SceneObject;
using SceneObjectPtr = std::shared_ptr<SceneObject>;

class SceneObject {
private:
  std::optional<std::shared_ptr<RenderPacket>> m_render_packet = std::nullopt;
  std::shared_ptr<RenderPacket> &create_render_packet(App &app);

  std::optional<SceneObjectPtr> m_parent = std::nullopt;
  std::vector<SceneObjectPtr> m_children;

  BoundingBox m_world_AABB;
  bool m_world_AABB_dirty = true;

  glm::mat4 m_model_matrix = glm::mat4(1.0f);

public:

  TransformComponent local_transform;
  PhysicsComponent physics;

  glm::mat4 get_local_transformation_mat();
  glm::mat4 get_world_transformation_mat();
  BoundingBox &get_world_AABB();
  void update_world_AABB();

  template<typename F>
  void with_render_packet(App& app, F&& f) {
      auto packet = create_render_packet(app);
      f(packet);
      m_render_packet = std::move(packet);
  }
  std::optional<std::shared_ptr<RenderPacket>> get_render_packet();

  void add_child(SceneObjectPtr child);

  auto begin() noexcept { return m_children.begin(); }
  auto end() noexcept { return m_children.end(); }
  auto begin() const noexcept { return m_children.begin(); }
  auto end() const noexcept { return m_children.end(); }
};
