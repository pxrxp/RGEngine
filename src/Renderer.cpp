#include "Renderer.h"
#include "Material.h"
#include "Scene.h"
#include "Shader.h"
#include <algorithm>
#include <memory>
#include <unordered_set>

#include "debug.h"

void
Renderer::add(std::shared_ptr<RenderPacket> render_packet)
{
  m_render_queue.push_back(render_packet);
}

void
Renderer::add_children(SceneObjectPtr root,
                       std::unordered_set<SceneObjectPtr>& set)
{
  set.insert(root);

  auto render_packet = root->get_render_packet();
  if (render_packet.has_value())
    add(render_packet.value());

  for (auto& child : *root) {
    if (!set.count(child))
      add_children(child, set);
  }
}

void
Renderer::submit(SceneObjectPtr scene)
{
  std::unordered_set<SceneObjectPtr> set;
  for (auto& child : *scene) {
    if (!set.count(child))
      add_children(child, set);
  }
}

void
Renderer::render()
{
  std::sort(
    m_render_queue.begin(),
    m_render_queue.end(),
    [](const auto& a, const auto& b) {
      if (a->priority != b->priority) {
        if (a->shader_program->get_id() != b->shader_program->get_id()) {
          return a->shader_program->get_id() < b->shader_program->get_id();
        }
        return a->material->get_id() < b->material->get_id();
      }
      return a->priority < b->priority;
    });

  std::shared_ptr<ShaderProgram> current_shader_program = nullptr;
  std::shared_ptr<Material> current_material = nullptr;

  for (auto& render_packet : m_render_queue) {
    if (!current_shader_program || render_packet->shader_program->get_id() !=
                                     current_shader_program->get_id()) {
      current_shader_program = render_packet->shader_program;
      current_shader_program->bind();
    }

    if (!current_material || render_packet->material != current_material) {
      if (current_material) {
        current_material->unbind();
      }
      current_material = render_packet->material;
      current_material->bind();
    }

    render_packet->render();
  }

  m_render_queue.clear();
}
