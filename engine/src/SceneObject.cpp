#include <algorithm>
#include <cmath>
#include <engine/App.h>
#include <engine/Mesh.h>
#include <engine/Scene.h>
#include <engine/SceneObject.h>
#include <engine/components/BoundingBox.h>
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
    const auto& local_AABB = m_render_packet.value()->mesh->get_local_AABB();
    const glm::vec3 mn = local_AABB.get_min();
    const glm::vec3 mx = local_AABB.get_max();

    // Cache world transform once instead of 8 times!
    const glm::mat4 world_mat = get_world_transformation_mat();

    const glm::vec3 corners[8] = { { mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z },
                                   { mn.x, mx.y, mn.z }, { mx.x, mx.y, mn.z },
                                   { mn.x, mn.y, mx.z }, { mx.x, mn.y, mx.z },
                                   { mn.x, mx.y, mx.z }, { mx.x, mx.y, mx.z } };

    glm::vec3 world_min(std::numeric_limits<float>::max());
    glm::vec3 world_max(std::numeric_limits<float>::lowest());

    for (int i = 0; i < 8; ++i) {
      glm::vec3 transformed =
        glm::vec3(world_mat * glm::vec4(corners[i], 1.0f));
      world_min = glm::min(world_min, transformed);
      world_max = glm::max(world_max, transformed);
    }

    m_world_AABB.set_min(world_min);
    m_world_AABB.set_max(world_max);
  }
  m_world_AABB.m_dirty = false;
}

BoundingBox&
SceneObject::get_world_AABB()
{
  if (m_local_transform->is_dirty() || m_world_AABB.is_dirty())
    update_world_AABB();

  return m_world_AABB;
}

void
SceneObject::add_child(std::shared_ptr<SceneObject> child)
{
  m_children.push_back(child);
  child->m_parent = this->shared_from_this();
}

void
SceneObject::remove_child(const std::shared_ptr<SceneObject>& child)
{
  const auto it =
    std::find_if(m_children.begin(), m_children.end(), [&](const auto& entry) {
      return entry == child;
    });
  if (it == m_children.end())
    return;

  if (auto parent = it->get()->m_parent.lock(); parent.get() == this)
    it->get()->m_parent.reset();
  m_children.erase(it);
}

SceneObjectStrongPtr
SceneObject::get_parent() const
{
  return m_parent.lock();
}