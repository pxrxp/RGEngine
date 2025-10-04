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
    setup_skybox(skybox);
    m_scene_ptr->add_child(skybox);

    auto cube = std::make_shared<SceneObject>();
    cube->physics.set_gravity(false);
    setup_cube(cube);
    m_scene_ptr->add_child(cube);

    auto axes = std::make_shared<SceneObject>();
    setup_axes(axes);
    m_scene_ptr->add_child(axes);
    debug_object(axes, "axes");

    // auto frustum = std::make_shared<SceneObject>();
    // setup_frustum(frustum);
    // m_scene_ptr->add_child(frustum);

    // auto objmodel = std::make_shared<SceneObject>();
    // setup_obj_model(objmodel);
    // m_scene_ptr->add_child(objmodel);
    // display_AABB(objmodel, true);
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
                                 .shaders = { "skybox.vert", "skybox.frag" } },
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

  void setup_axes(const SceneObjectStrongPtr& axes)
  {
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
                   [this]([[maybe_unused]] SceneObjectStrongPtr self,
                          RenderPacketStrongPtr packet) {
                     set_view_matrix(packet);
                     set_projection_matrix(packet);
                     set_model_matrix(packet,
                                      self->get_world_transformation_mat());
                     packet->mesh->draw();
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
                                 .shaders = { "AABB.vert", "AABB.frag" } },
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
