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
                      app.material_repo.create(), LOW_PRIORITY);

  skybox.mesh->setup<VertexSimple>({CUBE_APPLY_TO_VERTICES(LIST_ITEM)},
                                   {CUBE_APPLY_TO_FACES(LIST_HEAD, LIST_TAIL)});
  skybox.shader_program->load_shaders({"skybox.vert.glsl", "skybox.frag.glsl"});
  skybox.material->load(skybox.shader_program,
                        {{"skybox", GL_TEXTURE_CUBE_MAP}});

  skybox.render = [&] {
    skybox.shader_program->set_uniform("view",
                                       app.get_camera().get_view_matrix());
    skybox.shader_program->set_uniform(
        "proj", app.get_camera().get_projection_matrix());

    glDepthFunc(GL_LEQUAL);

    skybox.mesh->draw();

    glDepthFunc(GL_LESS);
  };

  RenderPacket cube(app.mesh_repo.create(), app.shader_program_repo.create(),
                    app.material_repo.create());

  cube.shader_program->load_shaders({"cube.vert.glsl", "cube.frag.glsl"});

  cube.mesh->setup<VertexColored>({{.position = 0.5f * glm::vec3(CUBE_VERT0),
                                    .normal = {-0.577f, -0.577f, -0.577f},
                                    .color = {0.000f, 0.000f, 0.000f}},
                                   {.position = 0.5f * glm::vec3(CUBE_VERT1),
                                    .normal = {0.577f, -0.577f, -0.577f},
                                    .color = {1.000f, 0.000f, 0.000f}},
                                   {.position = 0.5f * glm::vec3(CUBE_VERT2),
                                    .normal = {0.577f, 0.577f, -0.577f},
                                    .color = {1.000f, 1.000f, 0.000f}},
                                   {.position = 0.5f * glm::vec3(CUBE_VERT3),
                                    .normal = {-0.577f, 0.577f, -0.577f},
                                    .color = {0.000f, 1.000f, 0.000f}},
                                   {.position = 0.5f * glm::vec3(CUBE_VERT4),
                                    .normal = {-0.577f, -0.577f, 0.577f},
                                    .color = {0.000f, 0.000f, 1.000f}},
                                   {.position = 0.5f * glm::vec3(CUBE_VERT5),
                                    .normal = {0.577f, -0.577f, 0.577f},
                                    .color = {1.000f, 0.000f, 1.000f}},
                                   {.position = 0.5f * glm::vec3(CUBE_VERT6),
                                    .normal = {0.577f, 0.577f, 0.577f},
                                    .color = {1.000f, 1.000f, 1.000f}},
                                   {.position = 0.5f * glm::vec3(CUBE_VERT7),
                                    .normal = {-0.577f, 0.577f, 0.577f},
                                    .color = {0.000f, 1.000f, 1.000f}}},
                                  {CUBE_FACES});

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
    float sensitivity = app.get_camera().get_sensitivity();
    ImGui::Begin("Debug");
    ImGui::Text("DeltaTime: %f", app.get_delta_time());
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
