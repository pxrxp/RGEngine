#include "App.h"
#include "Renderer.h"
#include "definitions.h"
#include "util.h"

#include <glm/glm.hpp>

#ifndef NDEBUG
#include <imgui.h>
#endif

int main() {
  App app;
  app.init();
  app.get_camera().setup({2., 3., 2.}, {0., 0., 0.},
                         app.get_window().get_aspect_ratio(), 20.0f,
                         {0.05f, 0.05f, 0.05f});

  RenderPacket skybox(app.mesh_repo.create(), app.shader_program_repo.create(),
                      app.material_repo.create());
  skybox.shader_program->load_shaders({"skybox.vert.glsl", "skybox.frag.glsl"});
  skybox.mesh->setup<VertexSimple>({CUBE_APPLY_TO_VERTICES(LIST_ITEM)},
                                   {CUBE_APPLY_TO_FACES(LIST_HEAD, LIST_TAIL)});
  skybox.render = [&] {
    skybox.shader_program->set_uniform("view",
                                     app.get_camera().get_view_matrix());
    skybox.shader_program->set_uniform("proj",
                                     app.get_camera().get_projection_matrix());
    glDepthFunc(GL_LEQUAL);
    skybox.mesh->draw();
    glDepthFunc(GL_LESS);
  };

  RenderPacket cube(app.mesh_repo.create(), app.shader_program_repo.create(),
                    app.material_repo.create());

  cube.shader_program->load_shaders({"cube.vert.glsl", "cube.frag.glsl"});

  cube.mesh->setup<VertexColored>(
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

  cube.render = [&] {
    cube.shader_program->set_uniform("model", cube.mesh->get_model_matrix());
    cube.shader_program->set_uniform("view",
                                     app.get_camera().get_view_matrix());
    cube.shader_program->set_uniform("proj",
                                     app.get_camera().get_projection_matrix());
    cube.mesh->draw();
  };

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

    app.get_window().clear(COLOR_BLACK,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    app.get_renderer().add(cube);
    app.get_renderer().add(skybox);
    app.get_renderer().render();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
