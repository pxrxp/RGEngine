#include "Scene.h"
#include <optional>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "App.h"

std::shared_ptr<RenderPacket> &SceneObject::create_render_packet(App &app) {
  render_packet.emplace(std::make_shared<RenderPacket>(
      app.mesh_repo.create(), app.shader_program_repo.create(),
      app.material_repo.create()));

  return render_packet.value();
}

void SceneObject::rotate(const glm::vec3 &axis, float angle) {
  m_local_transform.orientation = glm::normalize(glm::angleAxis(angle, axis) * m_local_transform.orientation);
  m_changed = true;
}
void SceneObject::translate(const glm::vec3 &offset) {
  m_local_transform.position += offset;
  m_changed = true;
}
void SceneObject::scale(const glm::vec3 &factors) {
  m_local_transform.scale_factors *= factors;
  m_changed = true;
}

glm::mat4 SceneObject::get_local_transform() {
  if (!m_changed)
    return m_model_matrix;

  glm::mat4 T = glm::translate(glm::mat4(1.f), m_local_transform.position);
  glm::mat4 R = glm::mat4_cast(m_local_transform.orientation);
  glm::mat4 S = glm::scale(glm::mat4(1.f), m_local_transform.scale_factors);

  m_model_matrix = T * R * S;
  m_changed = false;
  return m_model_matrix;
}

glm::mat4 SceneObject::get_world_transform() {
  if (m_parent.has_value())
    return m_parent.value()->get_world_transform() * get_local_transform();

  return get_local_transform();
}

void SceneObject::add_child(std::shared_ptr<SceneObject> child) {
  m_children.push_back(child);
  child->m_parent = std::shared_ptr<SceneObject>(this);
}
