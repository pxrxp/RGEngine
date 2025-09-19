#include "App.h"
#include "Colors.h"
#include "Mesh.h"
#include "Shader.h"
#include "util.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#ifndef NDEBUG
#include <imgui.h>
#endif

int main() {
  App app;
  app.init_window();

  ShaderProgram program;
  program.load_shaders({"triangle.vert.glsl", "triangle.frag.glsl"});

  Mesh cube;
  cube.setup(
      {
          -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f,
          0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f,

          -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
          0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,
      },
      {// Front
       0, 1, 2, 0, 2, 3,

       // Back
       5, 4, 7, 5, 7, 6,

       // Left
       4, 0, 3, 4, 3, 7,

       // Right
       1, 5, 6, 1, 6, 2,

       // Top
       3, 2, 6, 3, 6, 7,

       // Bottom
       4, 5, 1, 1, 0, 4

      },
      {3});

  glm::mat4 model(1.f);
  glm::mat4 view(1.f);
  glm::mat4 proj = glm::perspective(
      glm::radians(60.f), app.get_window().get_aspect_ratio(), 0.1f, 100.f);

  glm::vec3 eye(2.f, 3.f, 1.f);
  glm::vec3 target(0.f, 0.f, 0.0f);

  glm::vec3 forward, up(0.f, 1.f, 0.f), right;
  glm::mat4 to_origin(1.f), to_axes(1.f);

  forward = glm::normalize(target - eye);
  right = glm::normalize(glm::cross(forward, up));
  up = glm::cross(right, forward);

  to_origin = glm::translate(to_origin, -eye);
  to_axes[0] = glm::vec4(right, 0.f);
  to_axes[1] = glm::vec4(up, 0.f);
  to_axes[2] = glm::vec4(-forward, 0.f);
  to_axes = glm::transpose(to_axes);

  view = to_axes * to_origin;

  // view = glm::lookAt(eye, target, glm::vec3(0.f, 1.f, 0.f));

#ifndef NDEBUG
  MatrixEditor model_editor(model), view_editor(view), proj_editor(proj);
#endif

  while (app.is_running()) {
    app.update_window();
    app.init_debug_gui();

    auto mouse = app.get_window().get_mouse_pos_ndc();

#ifndef NDEBUG
    ImGui::Text("Mouse: %f, %f", mouse.first, mouse.second);
    ImGui::Text("Forward: %f, %f, %f", forward.x, forward.y, forward.z);
    ImGui::Text("Up: %f, %f, %f", up.x, up.y, up.z);
    ImGui::Text("Right: %f, %f, %f", right.x, right.y, right.z);

    if (ImGui::CollapsingHeader("Model"))
      model_editor.Draw("Model");
    if (ImGui::CollapsingHeader("View"))
      view_editor.Draw("View");
    if (ImGui::CollapsingHeader("Projection"))
      proj_editor.Draw("Projection");
#endif

    app.get_window().clear(COLOR_MAROON,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    program.set_uniform("model", model);
    program.set_uniform("view", view);
    program.set_uniform("proj", proj);
    cube.draw();
    program.unbind();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
