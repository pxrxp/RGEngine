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

  glm::vec3 eye(2.f, 3.f, 1.f);
  glm::vec3 target(0.f, 0.f, 0.0f);

  glm::mat4 model(1.f);
  glm::mat4 view = glm::lookAt(eye, target, {0.f, 1.f, 0.f});
  glm::mat4 proj = glm::perspective(
      glm::radians(60.f), app.get_window().get_aspect_ratio(), 0.1f, 100.f);

#ifndef NDEBUG
  MatrixEditors all_matrix_editors;
  all_matrix_editors.Add("Model", model);
  all_matrix_editors.Add("View", view);
  all_matrix_editors.Add("Projection", proj);
#endif

  while (app.is_running()) {
    app.update_window();
    app.init_debug_gui();

    auto mouse = app.get_window().get_mouse_pos();
    auto delta_mouse = app.get_window().get_delta_mouse();
    auto delta_yaw =
        static_cast<float>(10. * app.get_delta_time() * delta_mouse.x);
    auto delta_pitch =
        static_cast<float>(10. * app.get_delta_time() * delta_mouse.y);

    glm::mat4 rot;
    rot = glm::rotate(glm::mat4(1.f), delta_pitch, {1., 0., 0.});
    rot = glm::rotate(rot, delta_yaw, {0., 1., 0.});
    view = glm::inverse(rot) * view;

#ifndef NDEBUG
    ImGui::Begin("Debug");
    ImGui::Text("Mouse: %f, %f", mouse.x, mouse.y);
    ImGui::Text("DeltaMouse: %f, %f", delta_mouse.x, delta_mouse.y);
    ImGui::Text("DeltaTime: %f", app.get_delta_time());
    all_matrix_editors.Draw();
    ImGui::End();

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
