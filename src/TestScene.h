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

  TestScene(const TestScene&) = delete;
  TestScene& operator=(const TestScene&) = delete;

  TestScene(TestScene&&) = default;
  TestScene& operator=(TestScene&&) = delete;

  void init() override
  {
    Scene::init();

    auto skybox = std::make_shared<SceneObject>();
    setup_skybox(skybox, "skybox");
    m_scene_ptr->add_child(skybox);

    auto cube = std::make_shared<SceneObject>();
    cube->physics.set_gravity(false);
    setup_cube(cube);
    m_scene_ptr->add_child(cube);

    debug_object(cube, "cube");

    debug_camera();

    // auto objmodel = std::make_shared<SceneObject>();
    // setup_obj_model(objmodel);
    // m_scene_ptr->add_child(objmodel);
  }

private:
  void setup_cube(const SceneObjectStrongPtr& cube)
  {
    cube->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        MeshDescriptor<Vertex_PosColNorm, TriangleIndices>{
          .mesh_name = "colored_cube_mesh",
          .vertices = { {
            { 0.5f * glm::vec3(CUBE_VERT0),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
            { 0.5f * glm::vec3(CUBE_VERT1),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
            { 0.5f * glm::vec3(CUBE_VERT2),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
            { 0.5f * glm::vec3(CUBE_VERT3),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
            { 0.5f * glm::vec3(CUBE_VERT4),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
            { 0.5f * glm::vec3(CUBE_VERT5),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
            { 0.5f * glm::vec3(CUBE_VERT6),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
            { 0.5f * glm::vec3(CUBE_VERT7),
              { 1.f, 1.f, 1.f },
              { 0.f, 0.f, 0.f } },
          } },
          .indices = { CUBE_FACES },
          .draw_primitive = GL_TRIANGLES }
          .recalculate_normals()),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "colored_cube_material",
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
  }

  void setup_obj_model(const SceneObjectStrongPtr& objmodel)
  {
    objmodel->physics.set_gravity(false);
    objmodel->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        from_OBJ<Vertex_Pos, TriangleIndices>("Box.obj", GL_TRIANGLES)),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "debug_obj_material",
        .shader_program_desc = { .program_name = "AABB_program",
                                 .shaders = { "AABB.vert", "AABB.frag" } },
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
