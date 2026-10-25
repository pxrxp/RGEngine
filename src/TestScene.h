#pragma once
#include "App.h"
#include "Material.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "ShaderProgram.h"
#include "components/vertex_formats.h"
#include "repos/MaterialRepo.h"
#include "repos/MeshRepo.h"
#include "util/obj_model.h"
#include "util/opengl.h"

class TestScene : public Scene
{
public:
  TestScene(App& app)
    : Scene(app)
  {
  }

  void init() override
  {
    Scene::init();
    debug_camera();

    // 1. Skybox
    auto skybox = std::make_shared<SceneObject>();
    setup_skybox(skybox, "skybox");
    m_scene_ptr->add_child(skybox);

    // 2. Infinite Grid Floor
    auto plane = std::make_shared<SceneObject>();
    setup_plane(plane);
    m_scene_ptr->add_child(plane);
    debug_object(plane, "plane");

    // 3. Falling physics cubes of various colors
    spawn_cube({ 0.f, 8.f, -5.f }, { 0.9f, 0.4f, 0.2f }, true);    // Orange
    spawn_cube({ 2.f, 12.f, -6.f }, { 0.2f, 0.7f, 0.9f }, true);   // Cyan
    spawn_cube({ -2.f, 16.f, -4.f }, { 0.3f, 0.85f, 0.4f }, true); // Lime Green

    // 4. A static pedestal/box to walk around
    spawn_cube({ -4.f, 0.5f, -8.f }, { 0.85f, 0.85f, 0.85f }, false);
    spawn_cube({ 4.f, 1.0f, -8.f }, { 0.85f, 0.3f, 0.3f }, false);

    // 5. Load OBJ Model from file
    auto objmodel = std::make_shared<SceneObject>();
    setup_obj_model(objmodel, "cube.obj", { 0.0f, 1.0f, -10.0f });
    m_scene_ptr->add_child(objmodel);
  }

private:
  void spawn_cube(const glm::vec3& pos, const glm::vec3& color, bool gravity)
  {
    auto cube = std::make_shared<SceneObject>();
    cube->get_local_transform().set_position(pos).scale(glm::vec3(0.5f));
    cube->get_physics_component().set_gravity(gravity);

    cube->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        MeshDescriptor<Vertex_PosColNorm, TriangleIndices>{
          .mesh_name =
            "cube_mesh_" + std::to_string(pos.x) + "_" + std::to_string(pos.z),
          .vertices = { {
            { CUBE_VERT0, color, {} },
            { CUBE_VERT1, color, {} },
            { CUBE_VERT2, color, {} },
            { CUBE_VERT3, color, {} },
            { CUBE_VERT4, color, {} },
            { CUBE_VERT5, color, {} },
            { CUBE_VERT6, color, {} },
            { CUBE_VERT7, color, {} },
          } },
          .indices = { CUBE_FACES },
          .draw_primitive = GL_TRIANGLES }
          .recalculate_normals()),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "cube_material_" + std::to_string(pos.x) + "_" +
                         std::to_string(pos.z),
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = {},
        .uniforms = {} }),

      capture_weak(
        cube, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        }));

    m_scene_ptr->add_child(cube);
  }

  void setup_plane(const SceneObjectStrongPtr& plane)
  {
    const auto far = m_app_cache.get_camera().get_far_plane();
    plane->get_local_transform()
      .set_position({ 0.0f, 0.0f, 0.0f })
      .scale({ far, far, far })
      .rotate_deg(AXIS_X, -90.f);
    plane->get_physics_component().set_gravity(false).set_mass(INFINITY);

    plane->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        MeshDescriptor<Vertex_PosColNorm, TriangleIndices>{
          .mesh_name = "ground_plane_mesh",
          .vertices = { {
            { QUAD_VERT0, { 1.f, 1.f, 1.f }, {} },
            { QUAD_VERT1, { 1.f, 1.f, 1.f }, {} },
            { QUAD_VERT2, { 1.f, 1.f, 1.f }, {} },
            { QUAD_VERT3, { 1.f, 1.f, 1.f }, {} },
          } },
          .indices = { QUAD_FACES },
          .draw_primitive = GL_TRIANGLES }
          .recalculate_normals()),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "ground_material",
        .shader_program_desc = { .program_name = "ground_program",
                                 .shaders = { "ground.vert", "ground.frag" } },
        .texture_desc_list = {},
        .uniforms = {} }),

      capture_weak(
        plane, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        }));
  }

  void setup_obj_model(const SceneObjectStrongPtr& objmodel,
                       const std::string& filename,
                       const glm::vec3& pos)
  {
    objmodel->get_local_transform().set_position(pos);
    objmodel->get_physics_component().set_gravity(false);

    objmodel->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        from_OBJ<Vertex_PosColNorm, TriangleIndices>(filename, GL_TRIANGLES)),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "obj_model_material_" + filename,
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = {},
        .uniforms = {} }),

      capture_weak(
        objmodel,
        [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        }));
  }
};
