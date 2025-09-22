#pragma once
#include "App.h"
#include "Scene.h"
#include "definitions.h"

class TestScene {
  SceneObjectPtr m_scene_ptr;

public:
  TestScene(App &app) {

    m_scene_ptr = std::make_shared<SceneObject>();

    SceneObjectPtr skybox = std::make_shared<SceneObject>();
    auto &skybox_render_packet = skybox->create_render_packet(app);

    skybox_render_packet->mesh->load<VertexSimple>(
        {CUBE_APPLY_TO_VERTICES(LIST_ITEM)},
        {CUBE_APPLY_TO_FACES(LIST_HEAD, LIST_TAIL)});
    skybox_render_packet->shader_program->load(
        {"skybox.vert.glsl", "skybox.frag.glsl"});
    skybox_render_packet->material->load(skybox_render_packet->shader_program,
                                         {{"skybox",
                                           {.target = GL_TEXTURE_CUBE_MAP,
                                            .wrap_s = GL_CLAMP_TO_EDGE,
                                            .wrap_t = GL_CLAMP_TO_EDGE,
                                            .wrap_r = GL_CLAMP_TO_EDGE}}});

    skybox_render_packet->render = [&app, skybox_render_packet, skybox] {
      skybox_render_packet->shader_program->set_uniform(
          "view", app.get_camera().get_view_matrix());
      skybox_render_packet->shader_program->set_uniform(
          "proj", app.get_camera().get_projection_matrix());

      glDepthFunc(GL_LEQUAL);

      skybox_render_packet->mesh->draw();

      glDepthFunc(GL_LESS);
    };

    m_scene_ptr->add_child(skybox);

    SceneObjectPtr cube = std::make_shared<SceneObject>();
    auto &cube_render_packet = cube->create_render_packet(app);

    cube_render_packet->shader_program->load(
        {"cube.vert.glsl", "cube.frag.glsl"});

    cube_render_packet->mesh->load<VertexColored>(
        {{0.5f * glm::vec3(CUBE_VERT0),
          {-0.577f, -0.577f, -0.577f},
          {0.000f, 0.000f, 0.000f}},
         {0.5f * glm::vec3(CUBE_VERT1),
          {0.577f, -0.577f, -0.577f},
          {1.000f, 0.000f, 0.000f}},
         {0.5f * glm::vec3(CUBE_VERT2),
          {0.577f, 0.577f, -0.577f},
          {1.000f, 1.000f, 0.000f}},
         {0.5f * glm::vec3(CUBE_VERT3),
          {-0.577f, 0.577f, -0.577f},
          {0.000f, 1.000f, 0.000f}},
         {0.5f * glm::vec3(CUBE_VERT4),
          {-0.577f, -0.577f, 0.577f},
          {0.000f, 0.000f, 1.000f}},
         {0.5f * glm::vec3(CUBE_VERT5),
          {0.577f, -0.577f, 0.577f},
          {1.000f, 0.000f, 1.000f}},
         {0.5f * glm::vec3(CUBE_VERT6),
          {0.577f, 0.577f, 0.577f},
          {1.000f, 1.000f, 1.000f}},
         {0.5f * glm::vec3(CUBE_VERT7),
          {-0.577f, 0.577f, 0.577f},
          {0.000f, 1.000f, 1.000f}}},
        {CUBE_FACES});

    cube_render_packet->render = [&app, cube_render_packet, cube] {
      cube_render_packet->shader_program->set_uniform(
          "model", cube->get_world_transform());
      cube_render_packet->shader_program->set_uniform(
          "view", app.get_camera().get_view_matrix());
      cube_render_packet->shader_program->set_uniform(
          "proj", app.get_camera().get_projection_matrix());
      cube_render_packet->mesh->draw();
    };

    m_scene_ptr->add_child(cube);
  }

  SceneObjectPtr get_scene_ptr() const { return m_scene_ptr; }
};
