#include "Material.h"
#include "Mesh.h"
#include "Scene.h"
#include "ShaderProgram.h"
#include "util/editor.h"
#include <imgui.h>

#ifndef NDEBUG
void
Scene::debug_object(std::weak_ptr<SceneObject> weak_object, const char* header)
{
  if (auto object_s = weak_object.lock()) {
    auto aabb_visualizer = std::make_shared<SceneObject>();
    aabb_visualizer->get_physics_component().set_gravity(false);

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

              if (ImGui::TreeNode("Transform Component")) {
                Util::draw_transform_component_editor(
                  object_to_visualize->get_local_transform(), header);
                ImGui::TreePop();
              }

              ImGui::Checkbox("Show AABB", &self->display_AABB);
              ImGui::Checkbox("Show axes", &object_to_visualize->display_axes);
              ImGui::Separator();

              if (ImGui::TreeNode("Physics Component")) {
                Util::draw_physics_component_editor(
                  object_to_visualize->get_physics_component(), header);
                ImGui::TreePop();
              }

              ImGui::PopID();
            }

            if (self->display_AABB) {
              const BoundingBox& aabb = object_to_visualize->get_world_AABB();
              const glm::vec3 dimensions = aabb.max - aabb.min;
              const glm::vec3 center = aabb.min + 0.5f * dimensions;

              self->get_local_transform().set_position(center);
              self->get_local_transform().set_scale(0.5f * dimensions);

              set_view_matrix(packet);
              set_projection_matrix(packet);
              set_model_matrix(packet, self->get_world_transformation_mat());
              packet->mesh->draw();
            }
          }
        }));

    axes->get_physics_component().set_gravity(false);
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

    axes->get_physics_component().set_gravity(false);
    object_s->add_child(axes);
    m_scene_ptr->add_child(aabb_visualizer);
  }
}

void
Scene::debug_camera()
{
  auto frustum_visualizer = std::make_shared<SceneObject>();
  frustum_visualizer->get_physics_component().set_gravity(false);
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

          auto player = camera.get_target_player();
          if (ImGui::TreeNode("Transport Component")) {
            Util::draw_transform_component_editor(player->get_local_transform(),
                                                  "Camera");
            if (player->get_local_transform().is_dirty()) {
              camera.reset_mouse_cache();
              camera.set_view_matrix_dirty();
            }
            ImGui::Separator();
            ImGui::TreePop();
          }

          ImGui::Checkbox("Show frustum", &self->visible);
          if (ImGui::Button("Update frustum"))
            dirty = true;
          ImGui::Separator();

          float fovy_deg = camera.get_fovy();
          if (ImGui::SliderFloat("FOV", &fovy_deg, 30.0f, 120.0f, "%.1f deg")) {
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

#endif

void
Scene::setup_skybox(const SceneObjectStrongPtr& skybox,
                    const std::string& texture_name)
{
  skybox->get_physics_component().set_gravity(false);
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
