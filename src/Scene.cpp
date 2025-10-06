#include "Scene.h"
#include "Material.h"
#include "Mesh.h"
#include "Renderer.h"
#include "SceneObject.h"
#include "ShaderProgram.h"
#include "util/editor.h"
#include <imgui.h>

void
resolve_penetration(SceneObject& a, SceneObject& b)
{
  // BoundingBox& aabb_a = a.get_world_AABB();
  // BoundingBox& aabb_b = b.get_world_AABB();

  // float overlap_x =
  //   std::min(aabb_a.max.x, aabb_b.max.x) - std::max(aabb_a.min.x,
  //   aabb_b.min.x);
  // float overlap_y =
  //   std::min(aabb_a.max.y, aabb_b.max.y) - std::max(aabb_a.min.y,
  //   aabb_b.min.y);
  // float overlap_z =
  //   std::min(aabb_a.max.z, aabb_b.max.z) - std::max(aabb_a.min.z,
  //   aabb_b.min.z);

  // glm::vec3 mtv;

  // if (overlap_x < overlap_y && overlap_x < overlap_z) {
  //   mtv = glm::vec3(overlap_x, 0, 0);
  // } else if (overlap_y < overlap_z) {
  //   mtv = glm::vec3(0, overlap_y, 0);
  // } else {
  //   mtv = glm::vec3(0, 0, overlap_z);
  // }

  // glm::vec3 direction =
  //   b.local_transform.get_position() - a.local_transform.get_position();
  // if (glm::dot(direction, mtv) < 0.0f) {
  //   mtv = -mtv;
  // }

  // if (a.physics.get_mass() > 0.0001 && b.physics.get_mass() > 0.0001) {
  //   a.local_transform.translate(-mtv * a.physics.get_mass() /
  //                               (a.physics.get_mass() +
  //                               b.physics.get_mass()));
  //   b.local_transform.translate(mtv * b.physics.get_mass() /
  //                               (a.physics.get_mass() +
  //                               b.physics.get_mass()));
  // }
}

void
resolve_velocity(SceneObject& a, SceneObject& b)
{
  // glm::vec3 center_A = a.local_transform.get_position();
  // glm::vec3 center_B = b.local_transform.get_position();

  // glm::vec3 direction_vector = center_B - center_A;
  // glm::vec3 collision_normal = glm::normalize(direction_vector);

  // glm::vec3 v_rel = b.physics.get_velocity() - a.physics.get_velocity();
  // float vel_along_normal = glm::dot(v_rel, collision_normal);

  // if (vel_along_normal > 0)
  //   return;

  // float e = std::min(a.physics.get_restitution(),
  // b.physics.get_restitution()); float j = -(1 + e) * vel_along_normal; j /=
  // (1 / a.physics.get_mass()) + (1 / b.physics.get_mass());

  // glm::vec3 impulse_vec = j * collision_normal;
  // a.physics.apply_linear_impulse(-impulse_vec);
  // b.physics.apply_linear_impulse(impulse_vec);
}

void
Scene::init()
{
  SceneObjectStrongPtr player = std::make_shared<SceneObject>();
  player->physics.set_gravity(false);
  m_scene_ptr->add_child(player);

  m_app_cache.get_camera().setup(player);
}

void
Scene::set_view_matrix(RenderPacketStrongPtr render_packet,
                       const char* view_uniform_name,
                       const glm::mat4& view_matrix)
{
  glm::mat4 V = (view_matrix != glm::mat4(1.f))
                  ? view_matrix
                  : m_app_cache.get_camera().get_view_matrix();

  render_packet->material->get_shader_program()->set_uniform(view_uniform_name,
                                                             V);
}

void
Scene::set_projection_matrix(RenderPacketStrongPtr render_packet,
                             const char* proj_uniform_name,
                             const glm::mat4& proj_matrix)
{
  glm::mat4 P = (proj_matrix != glm::mat4(1.f))
                  ? proj_matrix
                  : m_app_cache.get_camera().get_projection_matrix();

  render_packet->material->get_shader_program()->set_uniform(proj_uniform_name,
                                                             P);
}

void
Scene::set_model_matrix(RenderPacketStrongPtr render_packet,
                        const glm::mat4& model_matrix,
                        const char* uniform_name)
{
  render_packet->material->get_shader_program()->set_uniform(uniform_name,
                                                             model_matrix);
}

void
Scene::debug_object(std::weak_ptr<SceneObject> weak_object, const char* header)
{
  if (auto object_s = weak_object.lock()) {
    auto aabb_visualizer = std::make_shared<SceneObject>();
    aabb_visualizer->physics.set_gravity(false);

    aabb_visualizer->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(MeshDescriptor<Vertex_Pos, LineIndices>{
        .mesh_name = "AABB_visualizer_mesh",
        .vertices = { CUBE_VERTICES },
        .indices = { CUBE_EDGES },
        .draw_primitive = GL_LINES }),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "AABB_visualizer_material",
        .shader_program_desc = { .program_name = "AABB_program",
                                 .shaders = { "AABB.vert", "AABB.frag", "AABB.geo" } },
        .texture_desc_list = {},
        .uniforms = {{"u_line_thickness", 0.05f}} }),

      capture_weak(aabb_visualizer,
                   [this, weak_object, header](SceneObjectStrongPtr self,
                                               RenderPacketStrongPtr packet) {
                     if (auto object_to_visualize = weak_object.lock()) {

                       if (ImGui::CollapsingHeader(header)) {
                         Util::draw_transform_component_editor(
                           object_to_visualize->local_transform);

                         ImGui::Checkbox("Show AABB", &self->display_AABB);
                       }

                       if (self->display_AABB) {
                         const BoundingBox& aabb =
                           object_to_visualize->get_world_AABB();
                         const glm::vec3 dimensions = aabb.max - aabb.min;
                         const glm::vec3 center = aabb.min + 0.5f * dimensions;

                         self->local_transform.set_position(center);
                         self->local_transform.set_scale(0.5f * dimensions);

                         set_view_matrix(packet);
                         set_projection_matrix(packet);
                         set_model_matrix(packet,
                                          self->get_world_transformation_mat());
                         packet->mesh->draw();
                       }
                     }
                   }));

    m_scene_ptr->add_child(aabb_visualizer);
  }
}
void
Scene::collect_physics_objects_recursive(
  SceneObjectStrongPtr object,
  std::vector<SceneObjectStrongPtr>& objects)
{
  objects.push_back(object);
  for (auto& child : *object) {
    collect_physics_objects_recursive(child, objects);
  }
}

std::vector<SceneObjectStrongPtr>
Scene::get_all_physics_objects()
{
  std::vector<SceneObjectStrongPtr> objects;
  for (auto& child : *m_scene_ptr) {
    collect_physics_objects_recursive(child, objects);
  }
  return objects;
}

void
Scene::step_simulation(float fixed_step)
{
  std::vector<SceneObjectStrongPtr> objects = get_all_physics_objects();

  for (auto& object : objects) {
    if (object->physics.has_gravity()) {
      glm::vec3 gravity_force =
        glm::vec3(0.0f, -9.8f, 0.0f) * object->physics.get_mass();
      object->physics.apply_force(gravity_force);
    }
  }

  for (size_t i = 0; i < objects.size(); ++i) {
    for (size_t j = i + 1; j < objects.size(); ++j) {
      SceneObjectStrongPtr& a = objects[i];
      SceneObjectStrongPtr& b = objects[j];

      const BoundingBox& aabb_a = a->get_world_AABB();
      const BoundingBox& aabb_b = a->get_world_AABB();

      if (aabb_a && aabb_b && aabb_a.intersects(aabb_b)) {
        resolve_penetration(*a, *b);
        resolve_velocity(*a, *b);
      }
    }
  }

  for (auto& object : objects) {
    object->physics.integrate(fixed_step);

    object->local_transform.translate(object->physics.get_velocity() *
                                      fixed_step);

    float angle =
      glm::length(object->physics.get_angular_velocity()) * fixed_step;

    if (angle > 0.0001f) {
      glm::vec3 axis = glm::normalize(object->physics.get_angular_velocity());
      object->local_transform.rotate(axis, angle);
    }
  }

  for (auto& object : objects) {
    object->physics.clear_forces();
  }
}

void
Scene::update_physics()
{
  static double accumulator = 0.0;
  const double FIXED_TIMESTEP = 1.0 / 60.0;

  accumulator += m_app_cache.get_delta_time();

  while (accumulator >= FIXED_TIMESTEP) {
    step_simulation(FIXED_TIMESTEP);
    accumulator -= FIXED_TIMESTEP;
  }
}

void
Scene::submit_to_renderer()
{
  m_app_cache.get_renderer().submit(get_scene_ptr());
}

SceneObjectStrongPtr
Scene::get_scene_ptr() const
{
  return m_scene_ptr;
}
