#include "App.h"
#include "Mesh.h"
#include "Shader.h"
#include "definitions.h"
#include "util.h"

#include <glm/glm.hpp>

#ifndef NDEBUG
#include <imgui.h>
#endif

int main() {
  App app;
  app.init();

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
  app.get_camera().setup({2., 3., 2.}, {0., 0., 0.},
                         app.get_window().get_aspect_ratio(), 3.0f);

  glm::mat4 temp_view;
  glm::mat4 temp_proj;

#ifndef NDEBUG
  MatrixEditors all_matrix_editors;
  all_matrix_editors.add("Model", model);
  all_matrix_editors.add("View (readonly)", temp_view);
  all_matrix_editors.add("Projection (readonly)", temp_proj);
#endif

  while (app.is_running()) {
    app.update();
    app.init_debug_gui();

    auto mouse = app.get_window().get_mouse_pos();
    auto delta_mouse = app.get_window().get_delta_mouse();

    temp_view = app.get_camera().get_view_matrix();
    temp_proj = app.get_camera().get_projection_matrix();

#ifndef NDEBUG
    ImGui::Begin("Debug");
    ImGui::Text("Mouse: %f, %f", mouse.x, mouse.y);
    ImGui::Text("DeltaMouse: %f, %f", delta_mouse.x, delta_mouse.y);
    ImGui::Text("DeltaTime: %f", app.get_delta_time());
    all_matrix_editors.draw();
    ImGui::End();

#endif

    app.get_window().clear(COLOR_MAROON,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    program.set_uniform("model", model);
    program.set_uniform("view", app.get_camera().get_view_matrix());
    program.set_uniform("proj", app.get_camera().get_projection_matrix());
    cube.draw();
    program.unbind();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
