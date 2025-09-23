#pragma once
#include "App.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "components.h"
#include "definitions.h"

#include "debug.h"

class TestScene {
  SceneObjectPtr m_scene_ptr = std::make_shared<SceneObject>();

  void apply_physics(SceneObjectPtr object,
                     std::unordered_set<SceneObjectPtr> &set) {
    set.insert(object);

    if (object->physics.has_gravity()) {
      glm::vec3 gravity_force =
          glm::vec3(0.0f, -1.0f, 0.0f) * object->physics.get_mass();
      object->physics.apply_force(gravity_force);
    }

    float fixed_step = 0.1f;
    object->physics.integrate(fixed_step);
    object->physics.clear_forces();

    object->local_transform.translate(object->physics.get_velocity() *
                                        fixed_step);

    float angle =
        glm::length(object->physics.get_angular_velocity()) * fixed_step;

    if (angle > 0.0001f) {
      glm::vec3 axis = glm::normalize(object->physics.get_angular_velocity());
      object->local_transform.rotate(axis, angle);
    }

    for (auto &child : *object) {
      if (!set.count(child))
        apply_physics(child, set);
    }
  }

public:
  TestScene(App &app) {

    SceneObjectPtr player = std::make_shared<SceneObject>();
    player->local_transform.translate({0.f, 0.f, 5.f});
    player->physics = PhysicsComponent({.has_gravity = false});
    m_scene_ptr->add_child(player);

    app.get_camera().setup(player).look_at({0.f, 0.f, 0.f});

    SceneObjectPtr skybox = std::make_shared<SceneObject>();
    auto &skybox_render_packet = skybox->create_render_packet(app);
    skybox->physics = PhysicsComponent({.has_gravity = false});

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
    cube->physics = PhysicsComponent({.has_gravity = false});

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
          "model", cube->get_world_transformation_mat());
      cube_render_packet->shader_program->set_uniform(
          "view", app.get_camera().get_view_matrix());
      cube_render_packet->shader_program->set_uniform(
          "proj", app.get_camera().get_projection_matrix());
      cube_render_packet->mesh->draw();
    };

    m_scene_ptr->add_child(cube);
  }

  void update_physics(App &app) {
    static double accumulator = 0.0;
    const double FIXED_TIMESTEP = 1.0 / 60.0;

    accumulator += app.get_delta_time();

    while (accumulator >= FIXED_TIMESTEP) {
      std::unordered_set<SceneObjectPtr> set;
      for (auto &child : *m_scene_ptr) {
        if (!set.count(child))
          apply_physics(child, set);
      }
      accumulator -= FIXED_TIMESTEP;
    }
  }

  SceneObjectPtr get_scene_ptr() const { return m_scene_ptr; }
};
