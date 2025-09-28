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

  std::weak_ptr<SceneObject> m_parent;
  std::vector<SceneObjectStrongPtr> m_children;

  BoundingBox m_world_AABB;
  bool m_world_AABB_dirty = true;

  glm::mat4 m_model_matrix = glm::mat4(1.0f);

public:
  template<typename F>
  static auto capture_weak(std::shared_ptr<SceneObject> obj, F&& f)
  {
    auto weak_obj = obj->weak_from_this();
    return [weak_obj, f = std::forward<F>(f)] {
      if (auto self = weak_obj.lock())
        f(self);
    };
  }

  TransformComponent local_transform;
  PhysicsComponent physics;

  glm::mat4 get_local_transformation_mat();
  glm::mat4 get_world_transformation_mat();
  BoundingBox& get_world_AABB();
  void update_world_AABB();

  template<typename F>
  void with_render_packet(App& app, F&& f)
  {
    RenderPacketStrongPtr packet = create_render_packet(app);
    std::weak_ptr<RenderPacket> packet_ptr = packet;
    f(packet_ptr);
    m_render_packet = std::move(packet);
  }
  std::optional<RenderPacketStrongPtr> get_render_packet();

  void add_child(SceneObjectStrongPtr child);
  SceneObjectStrongPtr get_parent() const;

  auto begin() noexcept { return m_children.begin(); }
  auto end() noexcept { return m_children.end(); }
  auto begin() const noexcept { return m_children.begin(); }
  auto end() const noexcept { return m_children.end(); }
};
