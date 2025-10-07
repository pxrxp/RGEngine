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

    auto axes = std::make_shared<SceneObject>();

    aabb_visualizer->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        MeshDescriptor<Vertex_Pos, LineIndices>{ .mesh_name = "simple_cube",
                                                 .vertices = { CUBE_VERTICES },
                                                 .indices = { CUBE_EDGES },
                                                 .draw_primitive = GL_LINES }),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "simple_lines",
        .shader_program_desc = { .program_name = "AABB_program",
                                 .shaders = { "AABB.vert",
                                              "AABB.frag",
                                              "AABB.geo" } },
        .texture_desc_list = {},
        .uniforms = { { "u_line_thickness", 0.05f } } }),

      capture_weak(
        aabb_visualizer,
        [this, weak_object, header](SceneObjectStrongPtr self,
                                    RenderPacketStrongPtr packet) {
          if (auto object_to_visualize = weak_object.lock()) {

            if (ImGui::CollapsingHeader(header)) {
              ImGui::PushID(header);
              Util::draw_transform_component_editor(
                object_to_visualize->local_transform);

              ImGui::Checkbox("Show AABB", &self->display_AABB);
              ImGui::Checkbox("Show axes", &object_to_visualize->display_axes);
              ImGui::PopID();
            }

            if (self->display_AABB) {
              const BoundingBox& aabb = object_to_visualize->get_world_AABB();
              const glm::vec3 dimensions = aabb.max - aabb.min;
              const glm::vec3 center = aabb.min + 0.5f * dimensions;

              self->local_transform.set_position(center);
              self->local_transform.set_scale(0.5f * dimensions);

              set_view_matrix(packet);
              set_projection_matrix(packet);
              set_model_matrix(packet, self->get_world_transformation_mat());
              packet->mesh->draw();
            }
          }
        }));

    axes->physics.set_gravity(false);
    axes->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(MeshDescriptor<Vertex_Pos, LineIndices>{
        .mesh_name = "axes_mesh",
        .vertices = { ORIGIN, AXIS_X, AXIS_Y, AXIS_Z },
        .indices = { { 0, 1 }, { 0, 2 }, { 0, 3 } },
        .draw_primitive = GL_LINES }),
      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "axes_material",
        .shader_program_desc = { .program_name = "axes_program",
                                 .shaders = { "axes.vert",
                                              "axes.frag",
                                              "axes.geo" } },
        .texture_desc_list = {},
        .uniforms = { { "u_line_thickness", 0.05f } } }),
      capture_weak(axes,
                   [this, weak_object](SceneObjectStrongPtr self,
                                       RenderPacketStrongPtr packet) {
                     if (auto object_to_visualize = weak_object.lock()) {
                       if (object_to_visualize->display_axes) {
                         set_view_matrix(packet);
                         set_projection_matrix(packet);
                         set_model_matrix(packet,
                                          self->get_world_transformation_mat());
                         packet->mesh->draw();
                       }
                     }
                   }));

    axes->physics.set_gravity(false);
    object_s->add_child(axes);
    m_scene_ptr->add_child(aabb_visualizer);
  }
}

void
Scene::setup_skybox(const SceneObjectStrongPtr& skybox,
                    const std::string& texture_name)
{
  skybox->physics.set_gravity(false);
  skybox->set_render_packet(
    m_app_cache.mesh_repo.load_mesh(MeshDescriptor<Vertex_Pos, TriangleIndices>{
      .mesh_name = "skybox_mesh",
      .vertices = { CUBE_VERTICES },
      .indices = { CUBE_FACES },
      .draw_primitive = GL_TRIANGLES }),

    m_app_cache.material_repo.load_material(MaterialDescriptor{
      .material_name = "skybox_material",
      .shader_program_desc = { .program_name = "skybox_program",
                               .shaders = { "skybox.vert", "skybox.frag" } },
      .texture_desc_list = { { .texture_name = texture_name,
                               .target = GL_TEXTURE_CUBE_MAP,
                               .wrap_s = GL_CLAMP_TO_EDGE,
                               .wrap_t = GL_CLAMP_TO_EDGE,
                               .wrap_r = GL_CLAMP_TO_EDGE } },
      .uniforms = { { "u_skybox", texture_name } } }),

    capture_weak(skybox,
                 [this]([[maybe_unused]] SceneObjectStrongPtr self,
                        RenderPacketStrongPtr packet) {
                   auto view = glm::mat4(
                     glm::mat3(m_app_cache.get_camera().get_view_matrix()));
                   packet->material->get_shader_program()->set_uniform("u_view",
                                                                       view);
                   set_projection_matrix(packet);
                   glDepthFunc(GL_LEQUAL);
                   packet->mesh->draw();
                   glDepthFunc(GL_LESS);
                 }));
}

void
Scene::debug_camera()
{
  auto frustum_visualizer = std::make_shared<SceneObject>();
  frustum_visualizer->physics.set_gravity(false);
  frustum_visualizer->visible = false;

  frustum_visualizer->set_render_packet(
    m_app_cache.mesh_repo.load_mesh(
      MeshDescriptor<Vertex_Pos, LineIndices>{ .mesh_name = "simple_cube",
                                               .vertices = { CUBE_VERTICES },
                                               .indices = { CUBE_EDGES },
                                               .draw_primitive = GL_LINES }),

    m_app_cache.material_repo.load_material(MaterialDescriptor{
      .material_name = "simple_lines",
      .shader_program_desc = { .program_name = "AABB_program",
                               .shaders = { "AABB.vert",
                                            "AABB.frag",
                                            "AABB.geo" } },
      .texture_desc_list = {},
      .uniforms = { { "u_line_thickness", 0.05f } } }),

    capture_weak(
      frustum_visualizer,
      [this, dirty = true, cached_model = glm::mat4(1.0f)](
        SceneObjectStrongPtr self, RenderPacketStrongPtr packet) mutable {
        if (ImGui::CollapsingHeader("Camera")) {
          auto& camera = m_app_cache.get_camera();

          if (ImGui::Button("Update frustum"))
            dirty = true;

          ImGui::Checkbox("Show frustum", &self->visible);
          ImGui::Separator();

          float fovy_deg = camera.get_fovy();
          if (ImGui::SliderFloat("FOV", &fovy_deg, 30.0f, 120.0f, "%.1f deg"))
          {
              camera.set_fovy(fovy_deg);
          }

          ImGui::Separator();
          float near_plane = camera.get_near_plane();
          float far_plane = camera.get_far_plane();

          if (ImGui::DragFloat(
                "Near Plane", &near_plane, 0.01f, 0.01f, far_plane - 0.01f)) {
            camera.set_near_plane(near_plane);
          }

          if (ImGui::DragFloat(
                "Far Plane", &far_plane, 0.1f, near_plane + 0.01f, 1000.0f)) {
            camera.set_far_plane(far_plane);
          }

          ImGui::Separator();

          float speed = camera.get_speed();
          float sensitivity = camera.get_sensitivity();

          if (ImGui::SliderFloat("Speed", &speed, 0.1f, 50.0f)) {
            camera.set_speed(speed);
          }

          if (ImGui::SliderFloat(
                "Sensitivity", &sensitivity, 0.01f, 1.0f, "%.3f")) {
            camera.set_sensitivity(sensitivity);
          }

          ImGui::Separator();

          glm::vec3 forward = camera.get_forward();
          glm::vec3 right = camera.get_right();
          glm::vec3 up = camera.get_up();

          ImGui::Text(
            "Forward: (%.3f, %.3f, %.3f)", forward.x, forward.y, forward.z);
          ImGui::Text("Right:   (%.3f, %.3f, %.3f)", right.x, right.y, right.z);
          ImGui::Text("Up:      (%.3f, %.3f, %.3f)", up.x, up.y, up.z);
        }
        if (self->visible) {
          if (dirty) {
            cached_model =
              glm::inverse(m_app_cache.get_camera().get_projection_matrix() *
                           m_app_cache.get_camera().get_view_matrix());
            dirty = false;
          }

          set_model_matrix(packet,
                           self->get_world_transformation_mat() * cached_model);
          set_view_matrix(packet);
          set_projection_matrix(packet);
          packet->mesh->draw();
        }
      }));
  m_scene_ptr->add_child(frustum_visualizer);
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
