#include "App.h"
#include "Mesh.h"
#include "Renderer.h"
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
  program.load_shaders({"cube.vert.glsl", "cube.frag.glsl"});

  Mesh cube;
  cube.setup<VertexColored>(
      {// Vertex 0
       {.position = {-0.500f, -0.500f, -0.500f},
        .normal = {-0.577f, -0.577f, -0.577f},
        .color = {0.000f, 0.000f, 0.000f}},
       // Vertex 1
       {.position = {0.500f, -0.500f, -0.500f},
        .normal = {0.577f, -0.577f, -0.577f},
        .color = {1.000f, 0.000f, 0.000f}},
       // Vertex 2
       {.position = {0.500f, 0.500f, -0.500f},
        .normal = {0.577f, 0.577f, -0.577f},
        .color = {1.000f, 1.000f, 0.000f}},
       // Vertex 3
       {.position = {-0.500f, 0.500f, -0.500f},
        .normal = {-0.577f, 0.577f, -0.577f},
        .color = {0.000f, 1.000f, 0.000f}},
       // Vertex 4
       {.position = {-0.500f, -0.500f, 0.500f},
        .normal = {-0.577f, -0.577f, 0.577f},
        .color = {0.000f, 0.000f, 1.000f}},
       // Vertex 5
       {.position = {0.500f, -0.500f, 0.500f},
        .normal = {0.577f, -0.577f, 0.577f},
        .color = {1.000f, 0.000f, 1.000f}},
       // Vertex 6
       {.position = {0.500f, 0.500f, 0.500f},
        .normal = {0.577f, 0.577f, 0.577f},
        .color = {1.000f, 1.000f, 1.000f}},
       // Vertex 7
       {.position = {-0.500f, 0.500f, 0.500f},
        .normal = {-0.577f, 0.577f, 0.577f},
        .color = {0.000f, 1.000f, 1.000f}}},
      {// Front
       {0, 1, 2},
       {0, 2, 3},
       // Back
       {5, 4, 7},
       {5, 7, 6},
       // Left
       {4, 0, 3},
       {4, 3, 7},
       // Right
       {1, 5, 6},
       {1, 6, 2},
       // Top
       {3, 2, 6},
       {3, 6, 7},
       // Bottom
       {4, 5, 1},
       {1, 0, 4}});

  app.get_camera().setup({2., 3., 2.}, {0., 0., 0.},
                         app.get_window().get_aspect_ratio(), 20.0f,
                         {0.05f, 0.05f, 0.05f});
#ifndef NDEBUG
  MatrixEditors all_matrix_editors;
  // all_matrix_editors.add("Model", model);
#endif

  while (app.is_running()) {
    app.update();
    app.init_debug_gui();

#ifndef NDEBUG
    ImGui::Begin("Debug");
    ImGui::Text("DeltaTime: %f", app.get_delta_time());
    all_matrix_editors.draw();
    ImGui::End();
#endif

    app.get_window().clear(COLOR_MAROON,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // program.bind();
    // program.set_uniform("model", cube.get_model_matrix());
    // program.set_uniform("view", app.get_camera().get_view_matrix());
    // program.set_uniform("proj", app.get_camera().get_projection_matrix());
    // cube.draw();
    // program.unbind();
    app.get_renderer().render();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
