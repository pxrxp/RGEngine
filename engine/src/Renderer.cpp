#include <engine/Renderer.h>
#include <engine/Material.h>
#include <engine/SceneObject.h>
#include <engine/ShaderProgram.h>
#include <algorithm>
#include <memory>
#include <unordered_set>

void
Renderer::add(RenderPacketStrongPtr render_packet)
{
  m_render_queue.push_back(render_packet);
}

void
Renderer::add_children(SceneObjectStrongPtr root,
                       std::unordered_set<SceneObjectStrongPtr>& set)
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
Renderer::submit(SceneObjectStrongPtr scene)
{
  std::unordered_set<SceneObjectStrongPtr> set;
  for (auto& child : *scene) {
    if (!set.count(child))
      add_children(child, set);
  }
}

void
Renderer::render()
{
  std::sort(m_render_queue.begin(),
            m_render_queue.end(),
            [](const auto& a, const auto& b) {
              if (a->priority != b->priority)
                return a->priority < b->priority;

              const auto a_shader = a->material->get_shader_program()->get_id();
              const auto b_shader = b->material->get_shader_program()->get_id();

              if (a_shader != b_shader)
                return a_shader < b_shader;

              return a->material->get_id() < b->material->get_id();
            });

  std::shared_ptr<ShaderProgram> current_shader_program = nullptr;
  std::shared_ptr<Material> current_material = nullptr;

  for (auto& render_packet : m_render_queue) {
    if (!current_shader_program ||
        render_packet->material->get_shader_program()->get_id() !=
          current_shader_program->get_id()) {
      current_shader_program = render_packet->material->get_shader_program();
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

  // The last material is not followed by another material transition, so
  // explicitly release its texture slots at the end of the frame.
  if (current_material) {
    current_material->unbind();
  }

  m_render_queue.clear();
}

void
Renderer::clear()
{
  m_render_queue.clear();
}
