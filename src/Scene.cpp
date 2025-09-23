#include "Scene.h"
#include "App.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>

#include "components.h"
#include "debug.h"

std::shared_ptr<RenderPacket> &SceneObject::create_render_packet(App &app) {
  render_packet.emplace(std::make_shared<RenderPacket>(
      app.mesh_repo.create(), app.shader_program_repo.create(),
      app.material_repo.create()));

  return render_packet.value();
}

glm::mat4 SceneObject::get_local_transform() {
  if (m_dirty) {
    m_model_matrix = local_transform.get_model_matrix();

    if (render_packet.has_value()) {
      auto m_local_AABB = render_packet.value()->mesh->get_local_AABB();
      std::vector<glm::vec3> local_AABB_corners = {
        {m_local_AABB.min.x, m_local_AABB.min.y, m_local_AABB.min.z},
        {m_local_AABB.min.x, m_local_AABB.min.y, m_local_AABB.max.z},
        {m_local_AABB.min.x, m_local_AABB.max.y, m_local_AABB.min.z},
        {m_local_AABB.min.x, m_local_AABB.max.y, m_local_AABB.max.z},
        {m_local_AABB.max.x, m_local_AABB.min.y, m_local_AABB.min.z},
        {m_local_AABB.max.x, m_local_AABB.min.y, m_local_AABB.max.z},
        {m_local_AABB.max.x, m_local_AABB.max.y, m_local_AABB.min.z},
        {m_local_AABB.max.x, m_local_AABB.max.y, m_local_AABB.max.z},
      };
      m_world_AABB = BoundingBox(local_AABB_corners);
    }
  }

  return m_model_matrix;
}

glm::mat4 SceneObject::get_world_transform() {
  if (m_parent.has_value())
    return m_parent.value()->get_world_transform() * get_local_transform();

  return get_local_transform();
}

BoundingBox& SceneObject::get_world_AABB() {
  return m_world_AABB;
}

void SceneObject::add_child(std::shared_ptr<SceneObject> child) {
  m_children.push_back(child);
  child->m_parent = std::shared_ptr<SceneObject>(this);
}
