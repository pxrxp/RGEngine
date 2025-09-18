#include "App.h"
#include "Colors.h"
#include "Mesh.h"
#include "Shader.h"
#include "util.h"
#include <glm/trigonometric.hpp>
#include <vector>

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
        0.f, 0.f, -1.f,
        1.f, 0.f, -1.f,
        1.f, 1.f, -1.f,
        0.f, 1.f, -1.f,

        0.f, 0.f, -2.f,
        1.f, 0.f, -2.f,
        1.f, 1.f, -2.f,
        0.f, 1.f, -2.f
      },
      {
        // Front
        0,1,2,
        0,2,3,

        // Back
        5,4,7,
        5,7,6,

        // Left
        0,4,7,
        0,7,1,

        // Right
        1,5,6,
        1,6,2,

        // Top
        1,2,6,
        1,6,7,

        // Bottom
        4,0,5,
        0,1,5
     
      }, {3});

  glm::mat4 model(1.f), view(1.f), proj(1.f);
  #ifndef NDEBUG
  MatrixEditor model_editor(model), view_editor(view), proj_editor(proj);
  #endif

  while (app.is_running()) {
    app.update_window();
    app.init_debug_gui();

    #ifndef NDEBUG
    if (ImGui::CollapsingHeader("Model")) model_editor.Draw("Model");
    if (ImGui::CollapsingHeader("View")) view_editor.Draw("View");
    if (ImGui::CollapsingHeader("Projection")) proj_editor.Draw("Projection");
    #endif

    app.get_window().clear(COLOR_MAROON, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
