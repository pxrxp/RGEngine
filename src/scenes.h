#pragma once
#include "App.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "components/PhysicsComponent.h"
#include "components/TransformComponent.h"
#include "definitions.h"

class TestScene : public Scene
{
public:
  TestScene(App& app)
    : Scene(app)
  {

    auto skybox = std::make_shared<SceneObject>();
    skybox->physics.set_gravity(false);

    skybox->with_render_packet(app, [=](auto packet) {
      packet->mesh->template load<SimpleVertex, TriangleIndices>(
        { CUBE_VERTICES }, { CUBE_FACES });
      packet->shader_program->load({ "skybox.vert.glsl", "skybox.frag.glsl" });
      packet->material->load(packet->shader_program,
                             { { "skybox",
                                 { .target = GL_TEXTURE_CUBE_MAP,
                                   .wrap_s = GL_CLAMP_TO_EDGE,
                                   .wrap_t = GL_CLAMP_TO_EDGE,
                                   .wrap_r = GL_CLAMP_TO_EDGE } } });

      packet->render = capture_weak(skybox, [=](auto self) {
        set_view_proj_matrix(packet);
        glDepthFunc(GL_LEQUAL);
        packet->mesh->draw();
        glDepthFunc(GL_LESS);
      });
    });

    m_scene_ptr->add_child(skybox);

    auto cube = std::make_shared<SceneObject>();
    cube->physics.set_gravity(true);

    cube->with_render_packet(app, [=](auto packet) {
      packet->shader_program->load({ "cube.vert.glsl", "cube.frag.glsl" });
      packet->mesh->template load<ColoredVertex, TriangleIndices>(
        { { 0.5f * glm::vec3(CUBE_VERT0),
            { -0.577f, -0.577f, -0.577f },
            { 0, 0, 0 } },
          { 0.5f * glm::vec3(CUBE_VERT1),
            { 0.577f, -0.577f, -0.577f },
            { 1, 0, 0 } },
          { 0.5f * glm::vec3(CUBE_VERT2),
            { 0.577f, 0.577f, -0.577f },
            { 1, 1, 0 } },
          { 0.5f * glm::vec3(CUBE_VERT3),
            { -0.577f, 0.577f, -0.577f },
            { 0, 1, 0 } },
          { 0.5f * glm::vec3(CUBE_VERT4),
            { -0.577f, -0.577f, 0.577f },
            { 0, 0, 1 } },
          { 0.5f * glm::vec3(CUBE_VERT5),
            { 0.577f, -0.577f, 0.577f },
            { 1, 0, 1 } },
          { 0.5f * glm::vec3(CUBE_VERT6),
            { 0.577f, 0.577f, 0.577f },
            { 1, 1, 1 } },
          { 0.5f * glm::vec3(CUBE_VERT7),
            { -0.577f, 0.577f, 0.577f },
            { 0, 1, 1 } } },
        { CUBE_FACES });

      packet->render = capture_weak(cube, [=](auto self) {
        set_view_proj_matrix(packet);
        set_model_matrix(packet, self->get_world_transformation_mat());
        packet->mesh->draw();
      });
    });

    cube->local_transform.scale({ 2.f, 1.f, 3.f });
    cube->local_transform.rotate(AXIS_Y, glm::radians(45.f));
    cube->local_transform.rotate(AXIS_X, glm::radians(60.f));
    cube->local_transform.rotate(AXIS_Z, glm::radians(30.f));

    m_scene_ptr->add_child(cube);
    display_AABB(cube, true);
  }
};
