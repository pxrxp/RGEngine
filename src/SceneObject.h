#pragma once
#include "Renderer.h"
#include "components/BoundingBox.h"
#include "components/PhysicsComponent.h"
#include "components/TransformComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>

class SceneObject;
using SceneObjectStrongPtr = std::shared_ptr<SceneObject>;
using SceneObjectWeakPtr = std::weak_ptr<SceneObject>;

class SceneObject : public std::enable_shared_from_this<SceneObject>
{
private:
  std::optional<RenderPacketStrongPtr> m_render_packet = std::nullopt;
  RenderPacketStrongPtr& create_render_packet(App& app);

  SceneObjectWeakPtr m_parent;
  std::vector<SceneObjectStrongPtr> m_children;

  BoundingBox m_world_AABB;
  bool m_world_AABB_dirty = true;

  glm::mat4 m_model_matrix = glm::mat4(1.0f);

public:
  TransformComponent local_transform;
  PhysicsComponent physics;

  glm::mat4 get_local_transformation_mat();
  glm::mat4 get_world_transformation_mat();
  BoundingBox& get_world_AABB();
  void update_world_AABB();

  #ifndef NDEBUG
  bool display_AABB = false;
  bool display_axes = false;
  #endif

  bool visible = true;

  template<typename... Args>
  void set_render_packet(Args&&... args)
  {
    m_render_packet = std::make_shared<RenderPacket>(std::forward<Args>(args)...);
  }

  std::optional<RenderPacketStrongPtr> get_render_packet();

  void add_child(SceneObjectStrongPtr child);
  SceneObjectStrongPtr get_parent() const;

  auto begin() noexcept { return m_children.begin(); }
  auto end() noexcept { return m_children.end(); }
  auto begin() const noexcept { return m_children.begin(); }
  auto end() const noexcept { return m_children.end(); }
};
