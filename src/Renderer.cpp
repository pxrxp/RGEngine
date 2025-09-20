#include "Renderer.h"
#include <algorithm>

void Renderer::add(RenderPacket render_packet) {
  render_queue.push_back(render_packet);
}

void Renderer::render() {
  std::sort(render_queue.begin(), render_queue.end(),
            [](const RenderPacket &a, const RenderPacket &b) {
              if (a.shader != b.shader) {
                return a.shader < b.shader;
              }
              return a.material < b.material;
            });

  GLuint current_shader = 0;
  unsigned int current_material = 0;

  for (auto &render_packet : render_queue) {
    if (render_packet.shader != current_shader) {
      glUseProgram(current_shader);
    }

    if (render_packet.material != current_material) {
      // current_material->unbind();
      current_material = render_packet.material;
      // current_material->bind();
    }

    render_packet.render();
  }
}
