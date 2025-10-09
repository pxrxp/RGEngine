#include "SceneObject.h"
#include "App.h"
#include "Mesh.h"
#include "Scene.h"
#include "components/BoundingBox.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>

SceneObject::SceneObject()
  : m_local_transform(std::make_unique<TransformComponent>())
  , m_physics(
      std::make_unique<PhysicsComponent>(*m_local_transform, Scene::FIXED_STEP))
{
}

std::optional<RenderPacketStrongPtr>
SceneObject::get_render_packet()
{
  return m_render_packet;
}

glm::mat4
SceneObject::get_local_transformation_mat()
{
  m_model_matrix = m_local_transform->get_model_matrix();
  return m_model_matrix;
}

glm::mat4
SceneObject::get_world_transformation_mat()
{
  if (m_local_transform->is_dirty())
    m_world_AABB_dirty = m_local_transform->is_dirty();

  glm::mat4 world_model_mat = get_local_transformation_mat();

  if (auto parent = get_parent()) {
    auto parent_mat = parent->get_world_transformation_mat();
    parent_mat[0] = glm::normalize(parent_mat[0]);
    parent_mat[1] = glm::normalize(parent_mat[1]);
    parent_mat[2] = glm::normalize(parent_mat[2]);
    world_model_mat = parent_mat * world_model_mat;
  }

  return world_model_mat;
}

void
SceneObject::update_world_AABB()
{
  if (m_render_packet.has_value()) {
    auto local_AABB = m_render_packet.value()->mesh->get_local_AABB();
    auto AABB_corners = local_AABB.corners();

    for (auto& corner : AABB_corners) {
      corner = { get_world_transformation_mat() *
                 glm::vec4(corner.position, 1.f) };
    }

    m_world_AABB = BoundingBox(AABB_corners);
  }
  m_world_AABB_dirty = false;
}

BoundingBox&
SceneObject::get_world_AABB()
{
  if (m_world_AABB_dirty)
    update_world_AABB();

  return m_world_AABB;
}

void
SceneObject::add_child(std::shared_ptr<SceneObject> child)
{
  m_children.push_back(child);
  child->m_parent = this->shared_from_this();
}

SceneObjectStrongPtr
SceneObject::get_parent() const
{
  return m_parent.lock();
}
