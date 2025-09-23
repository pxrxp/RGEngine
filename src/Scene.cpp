#include "Scene.h"
#include "App.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>

std::shared_ptr<RenderPacket> &SceneObject::create_render_packet(App &app) {
  render_packet.emplace(std::make_shared<RenderPacket>(
      app.mesh_repo.create(), app.shader_program_repo.create(),
      app.material_repo.create()));

  return render_packet.value();
}

glm::mat4 SceneObject::get_local_transform() {
  if (m_dirty)
    m_model_matrix = m_local_transform.get_model_matrix();
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
