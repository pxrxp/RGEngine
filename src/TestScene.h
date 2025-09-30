#pragma once
#include "App.h"
#include "Material.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "ShaderProgram.h"
#include "components/vertex_formats.h"
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
    setup_skybox(skybox);
    m_scene_ptr->add_child(skybox);

    auto cube = std::make_shared<SceneObject>();
    cube->physics.set_gravity(false);
    cube->local_transform.translate({ 5.f, 2.f, 4.f });
    setup_cube(cube);
    cube->local_transform.scale({ 2.f, 1.f, 3.f });
    cube->local_transform.rotate(AXIS_Y, glm::radians(45.f));
    cube->local_transform.rotate(AXIS_X, glm::radians(60.f));
    cube->local_transform.rotate(AXIS_Z, glm::radians(30.f));
    m_scene_ptr->add_child(cube);

    auto frustum = std::make_shared<SceneObject>();
    setup_frustum(frustum);
    m_scene_ptr->add_child(frustum);

    auto objmodel = std::make_shared<SceneObject>();
    setup_obj_model(objmodel);
    m_scene_ptr->add_child(objmodel);
    display_AABB(objmodel, true);
  }

private:
  void setup_skybox(const SceneObjectStrongPtr& skybox)
  {
    skybox->physics.set_gravity(false);
    skybox->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        MeshDescriptor<Vertex_Pos, TriangleIndices>{
          .mesh_name = "skybox_mesh",
          .vertices = { CUBE_VERTICES },
          .indices = { CUBE_FACES },
          .draw_primitive = GL_TRIANGLES }),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "skybox_material",
        .shader_program_desc = { .program_name = "skybox_program",
                                 .shaders = { "skybox.vert",
                                              "skybox.frag" } },
        .texture_desc_list = { { .texture_name = "skybox",
                                 .target = GL_TEXTURE_CUBE_MAP,
                                 .wrap_s = GL_CLAMP_TO_EDGE,
                                 .wrap_t = GL_CLAMP_TO_EDGE,
                                 .wrap_r = GL_CLAMP_TO_EDGE } },
        .uniforms = { { "u_skybox", "skybox" } } }),

      capture_weak(
        skybox,
        [this]([[maybe_unused]] SceneObjectStrongPtr self,
               RenderPacketStrongPtr packet) {
          auto view =
            glm::mat4(glm::mat3(m_app_cache.get_camera().get_view_matrix()));
          packet->material->get_shader_program()->set_uniform("u_view", view);
          set_projection_matrix(packet);
          glDepthFunc(GL_LEQUAL);
          packet->mesh->draw();
          glDepthFunc(GL_LESS);
        }));
  }

  void setup_cube(const SceneObjectStrongPtr& cube)
  {
    cube->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        MeshDescriptor<Vertex_PosColNorm, TriangleIndices>{
          .mesh_name = "colored_cube_mesh",
          .vertices = { {
            { 0.5f * glm::vec3(CUBE_VERT0),
              { 0, 0, 0 },
              { -0.577f, -0.577f, -0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT1),
              { 1, 0, 0 },
              { 0.577f, -0.577f, -0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT2),
              { 1, 1, 0 },
              { 0.577f, 0.577f, -0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT3),
              { 0, 1, 0 },
              { -0.577f, 0.577f, -0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT4),
              { 0, 0, 1 },
              { -0.577f, -0.577f, 0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT5),
              { 1, 0, 1 },
              { 0.577f, -0.577f, 0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT6),
              { 1, 1, 1 },
              { 0.577f, 0.577f, 0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT7),
              { 0, 1, 1 },
              { -0.577f, 0.577f, 0.577f } },
          } },
          .indices = { CUBE_FACES },
          .draw_primitive = GL_TRIANGLES }),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "colored_cube_material",
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert",
                                              "cube.frag" } },
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

  void setup_frustum(const SceneObjectStrongPtr& frustum)
  {
    frustum->physics.set_gravity(false);
    auto initial_frustum_vertices =
      m_app_cache.get_camera().get_frustum_worldspace();

    frustum->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(MeshDescriptor<Vertex_Pos, LineIndices>{
        .mesh_name = "frustum_lines_mesh",
        .vertices = initial_frustum_vertices,
        .indices = { CUBE_EDGES },
        .draw_primitive = GL_LINES }),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "debug_line_material",
        .shader_program_desc = { .program_name = "AABB_program",
                                 .shaders = { "AABB.vert",
                                              "AABB.frag" } },
        .texture_desc_list = {},
        .uniforms = {} }),

      capture_weak(
        frustum,
        [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_model_matrix(packet, self->get_world_transformation_mat());
          set_view_matrix(packet);
          set_projection_matrix(packet);
          packet->mesh->draw();
        }));
  }

  void setup_obj_model(const SceneObjectStrongPtr& objmodel)
  {
    objmodel->physics.set_gravity(false);
    objmodel->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(
        from_OBJ<Vertex_Pos, TriangleIndices>("cube.obj", GL_TRIANGLES)),

      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = "debug_obj_material",
        .shader_program_desc = { .program_name = "AABB_program",
                                 .shaders = { "AABB.vert",
                                              "AABB.frag" } },
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
