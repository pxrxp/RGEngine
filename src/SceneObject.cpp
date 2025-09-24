#include "Scene.h"
#include "App.h"
#include "components/BoundingBox.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>

#include "debug.h"

std::shared_ptr<RenderPacket> &SceneObject::create_render_packet(App &app) {
  render_packet.emplace(std::make_shared<RenderPacket>(
      app.mesh_repo.create(), app.shader_program_repo.create(),
      app.material_repo.create()));

  return render_packet.value();
}

glm::mat4 SceneObject::get_local_transformation_mat() {
  m_model_matrix = local_transform.get_model_matrix();
  return m_model_matrix;
}

glm::mat4 SceneObject::get_world_transformation_mat() {
  m_world_AABB_dirty = local_transform.is_dirty();

  glm::mat4 world_model_mat = get_local_transformation_mat();

  if (m_parent.has_value())
    world_model_mat =
        m_parent.value()->get_world_transformation_mat() * world_model_mat;

  return world_model_mat;
}

void SceneObject::update_world_AABB() {
  if (render_packet.has_value()) {
    auto local_AABB = render_packet.value()->mesh->get_local_AABB();
    auto AABB_corners = local_AABB.corners();

    for (auto &corner : AABB_corners) {
      corner = {get_world_transformation_mat() *
                glm::vec4(corner.position, 1.f)};
    }

    m_world_AABB = BoundingBox(AABB_corners);
  }
  m_world_AABB_dirty = false;
}

BoundingBox &SceneObject::get_world_AABB() {
  if (m_world_AABB_dirty)
    update_world_AABB();

  return m_world_AABB;
}

void SceneObject::add_child(std::shared_ptr<SceneObject> child) {
  m_children.push_back(child);
  child->m_parent = std::shared_ptr<SceneObject>(this);
}
