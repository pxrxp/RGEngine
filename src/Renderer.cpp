#include "Renderer.h"
#include "Material.h"
#include "Shader.h"
#include "util.h"
#include <algorithm>
#include <memory>

void Renderer::add(RenderPacket render_packet) {
  render_queue.push_back(render_packet);
}

void Renderer::render() {
  std::sort(render_queue.begin(), render_queue.end(),
            [](const RenderPacket &a, const RenderPacket &b) {
              if (a.priority != b.priority) {
                if (a.shader_program->get_id() != b.shader_program->get_id()) {
                  return a.shader_program->get_id() < b.shader_program->get_id();
                }
                return a.material->get_id() < b.material->get_id();
              }
              return a.priority < b.priority;
            });

  std::shared_ptr<ShaderProgram> current_shader_program = nullptr;
  std::shared_ptr<Material> current_material = nullptr;

  for (auto &render_packet : render_queue) {
    if (!current_shader_program || render_packet.shader_program->get_id() !=
                                       current_shader_program->get_id()) {
      current_shader_program = render_packet.shader_program;
      current_shader_program->bind();
    }

    if (!current_material || render_packet.material != current_material) {
      if (current_material) {
        current_material->unbind();
      }
      current_material = render_packet.material;
      current_material->bind();
    }

    render_packet.render();
  }

  render_queue.clear();
}
