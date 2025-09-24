#pragma once
#include "App.h"
#include "Renderer.h"
#include "SceneObject.h"
#include "components/BoundingBox.h"
#include "components/PhysicsComponent.h"
#include "components/TransformComponent.h"
#include "definitions.h"

#include "debug.h"
#include "util.h"

class Scene {
private:
  App &m_app_cache;

protected:
  SceneObjectPtr m_scene_ptr = nullptr;
  std::shared_ptr<SceneObject> m_player;

  void set_view_proj_matrix(std::shared_ptr<RenderPacket> render_packet,
                            const char *view_uniform_name = "view",
                            const char *proj_uniform_name = "proj",
                            const glm::mat4 &view_matrix = glm::mat4(1.f),
                            const glm::mat4 &proj_matrix = glm::mat4(1.f)) {

    glm::mat4 V = (view_matrix != glm::mat4(1.f))
                      ? view_matrix
                      : m_app_cache.get_camera().get_view_matrix();
    glm::mat4 P = (proj_matrix != glm::mat4(1.f))
                      ? proj_matrix
                      : m_app_cache.get_camera().get_projection_matrix();

    render_packet->shader_program->set_uniform(view_uniform_name, V);
    render_packet->shader_program->set_uniform(proj_uniform_name, P);
  }

  void set_model_matrix(std::shared_ptr<RenderPacket> render_packet,
                        const glm::mat4 &model_matrix = glm::mat4(1.f),
                        const char *uniform_name = "model") {
    render_packet->shader_program->set_uniform(uniform_name, model_matrix);
  }

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

  void display_AABB(std::shared_ptr<SceneObject> object,
                    bool show_controller = false) {

    SceneObjectPtr object_AABB = std::make_shared<SceneObject>();
    auto &object_AABB_render_packet =
        object_AABB->create_render_packet(m_app_cache);
    object_AABB->physics = PhysicsComponent({.has_gravity = false});
    object_AABB_render_packet->shader_program->load(
        {"AABB.vert.glsl", "AABB.frag.glsl"});
    object_AABB_render_packet->mesh->load<SimpleVertex, LineIndices>(
        {CUBE_VERTICES}, {CUBE_EDGES}, GL_LINES);
    object_AABB_render_packet->render = [this, object_AABB_render_packet,
                                         object, object_AABB, show_controller] {
      if (show_controller)
        draw_transform_component_editor(object->local_transform,
                                        "object Local Transform");

      glm::vec3 aabb_min = object->get_world_AABB().min;
      glm::vec3 aabb_max = object->get_world_AABB().max;
      glm::vec3 dimensions = aabb_max - aabb_min;
      glm::vec3 center = aabb_min + 0.5f * dimensions;

      object_AABB->local_transform.set_position(center);
      object_AABB->local_transform.set_scale(0.5f * dimensions);

      object_AABB_render_packet->shader_program->set_uniform(
          "model", object_AABB->get_world_transformation_mat());
      object_AABB_render_packet->shader_program->set_uniform(
          "view", m_app_cache.get_camera().get_view_matrix());
      object_AABB_render_packet->shader_program->set_uniform(
          "proj", m_app_cache.get_camera().get_projection_matrix());
      object_AABB_render_packet->mesh->draw();
    };

    m_scene_ptr->add_child(object_AABB);
  }

public:
  Scene(App &app)
      : m_scene_ptr(std::make_shared<SceneObject>()), m_app_cache(app) {

    SceneObjectPtr player = std::make_shared<SceneObject>();
    player->physics = PhysicsComponent({.has_gravity = false});
    m_scene_ptr->add_child(player);
    m_player = player;

    app.get_camera().setup(player);
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
