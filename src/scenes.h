#pragma once
#include "App.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "components/PhysicsComponent.h"
#include "components/TransformComponent.h"
#include "definitions.h"

class TestScene : Scene {
public:
  TestScene(App &app) : Scene(app) {

    auto skybox = std::make_shared<SceneObject>();
    auto skybox_render_packet = skybox->create_render_packet(app);
    skybox->physics = PhysicsComponent({.has_gravity = false});

    skybox_render_packet->mesh->load<SimpleVertex, TriangleIndices>(
        {CUBE_VERTICES}, {CUBE_FACES});
    skybox_render_packet->shader_program->load(
        {"skybox.vert.glsl", "skybox.frag.glsl"});
    skybox_render_packet->material->load(skybox_render_packet->shader_program,
                                         {{"skybox",
                                           {.target = GL_TEXTURE_CUBE_MAP,
                                            .wrap_s = GL_CLAMP_TO_EDGE,
                                            .wrap_t = GL_CLAMP_TO_EDGE,
                                            .wrap_r = GL_CLAMP_TO_EDGE}}});

    skybox_render_packet->render = [this, packet = skybox_render_packet] {
      set_view_proj_matrix(packet);
      glDepthFunc(GL_LEQUAL);
      packet->mesh->draw();
      glDepthFunc(GL_LESS);
    };

    m_scene_ptr->add_child(skybox);

    auto cube = std::make_shared<SceneObject>();
    auto cube_render_packet = cube->create_render_packet(app);
    cube->physics = PhysicsComponent({.has_gravity = false});

    cube_render_packet->shader_program->load(
        {"cube.vert.glsl", "cube.frag.glsl"});

    cube_render_packet->mesh->load<ColoredVertex, TriangleIndices>(
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

    cube->local_transform.scale({2.f, 1.f, 3.f});
    cube->local_transform.rotate(AXIS_Y, glm::radians(45.f));
    cube->local_transform.rotate(AXIS_X, glm::radians(60.f));
    cube->local_transform.rotate(AXIS_Z, glm::radians(30.f));

    cube_render_packet->render = [this, packet = cube_render_packet, cube] {
      set_view_proj_matrix(packet);
      set_model_matrix(packet,
                       cube->get_world_transformation_mat());
      packet->mesh->draw();
    };
    m_scene_ptr->add_child(cube);

    display_AABB(cube, true);
  }
};
