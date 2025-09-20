#include "App.h"
#include "Renderer.h"
#include "definitions.h"
// #include "png_image.h"
#include "util.h"

#include <glm/glm.hpp>
// #include <iostream>

#ifndef NDEBUG
#include <imgui.h>
#endif

int main() {
  App app;
  app.init();
  app.get_camera().setup({2., 3., 2.}, {0., 0., 0.},
                         app.get_window().get_aspect_ratio(), 20.0f,
                         {0.05f, 0.05f, 0.05f});

  // RenderPacket skybox(app.mesh_repo.create(), app.shader_program_repo.create(),
  //                     app.material_repo.create());
  // skybox.shader_program->load_shaders({"skybox.vert.glsl", "skybox.frag.glsl"});
  // skybox.mesh->setup<VertexSimple>({CUBE_APPLY_TO_VERTICES(LIST_ITEM)},
  //                                  {CUBE_APPLY_TO_FACES(LIST_HEAD, LIST_TAIL)});
  // // skybox.shader_program->set_texture_unit("skybox", 0, true);

  // GLuint cubemapTexture;
  // glGenTextures(1, &cubemapTexture);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  // glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // {
  //   std::vector<PNGImage> faces = {
  //       PNGImage("right.png"),  PNGImage("left.png"),  PNGImage("top.png"),
  //       PNGImage("bottom.png"), PNGImage("front.png"), PNGImage("back.png")};

  //   for (unsigned int i = 0; i < faces.size(); i++) {
  //     if (!faces[i].is_valid()) {
  //       std::cerr << "Invalid image: " << faces[i].file_name << std::endl;
  //     }
  //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
  //                  faces[i].width, faces[i].height, 0, GL_RGB, GL_UNSIGNED_BYTE,
  //                  faces[i].pixels.data());
  //   }
  // }

  // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  // skybox.render = [&] {
  //   skybox.shader_program->set_uniform("view",
  //                                      app.get_camera().get_view_matrix());
  //   skybox.shader_program->set_uniform(
  //       "proj", app.get_camera().get_projection_matrix());

  //   glDepthFunc(GL_LEQUAL);

  //   skybox.mesh->bind();

  //   glActiveTexture(GL_TEXTURE0);
  //   glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  //   skybox.mesh->draw(false);

  //   skybox.mesh->unbind();

  //   glDepthFunc(GL_LESS);
  // };

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
    cube.mesh->draw(true);
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

    app.get_window().clear(PALETTE_TEAL,
                           GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    app.get_renderer().add(cube);
    // app.get_renderer().add(skybox);
    app.get_renderer().render();

    app.render_debug_gui();
    app.get_window().swap_buffers();
  }
}
