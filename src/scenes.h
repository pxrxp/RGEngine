#pragma once
#include "App.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "components/PhysicsComponent.h"
#include "components/TransformComponent.h"
#include "components/vertex_formats.h"
#include "util/definitions.h"
#include "util/obj_model.h"

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

  void init() override;

private:
  void setup_skybox(const SceneObjectStrongPtr& skybox);
  void setup_cube(const SceneObjectStrongPtr& cube);
  void setup_frustum(const SceneObjectStrongPtr& frustum);
  void setup_objmodel(const SceneObjectStrongPtr& objmodel);
};

void
TestScene::init()
{
  Scene::init();

  auto skybox = std::make_shared<SceneObject>();
  setup_skybox(skybox);
  m_scene_ptr->add_child(skybox);

  auto cube = std::make_shared<SceneObject>();
  setup_cube(cube);
  m_scene_ptr->add_child(cube);

  auto frustum = std::make_shared<SceneObject>();
  setup_frustum(frustum);
  m_scene_ptr->add_child(frustum);

  auto objmodel = std::make_shared<SceneObject>();
  setup_objmodel(objmodel);
  m_scene_ptr->add_child(objmodel);
}

void
TestScene::setup_skybox(const SceneObjectStrongPtr& skybox)
{
  skybox->physics.set_gravity(false);
  skybox->with_render_packet(
    m_app_cache, [this, skybox](RenderPacketWeakPtr weak_packet) {
      if (auto packet = weak_packet.lock()) {
        packet->mesh->template load<Vertex_Pos, TriangleIndices>(
          { CUBE_VERTICES }, { CUBE_FACES }, GL_TRIANGLES);
        packet->shader_program->load(
          m_app_cache.shader_repo, { "skybox.vert.glsl", "skybox.frag.glsl" });
        packet->material->load(m_app_cache.tex_repo,
                               packet->shader_program,
                               { { "skybox",
                                   { .target = GL_TEXTURE_CUBE_MAP,
                                     .wrap_s = GL_CLAMP_TO_EDGE,
                                     .wrap_t = GL_CLAMP_TO_EDGE,
                                     .wrap_r = GL_CLAMP_TO_EDGE } } });

        packet->render = SceneObject::capture_weak(
          skybox, [this, weak_packet]([[maybe_unused]] SceneObjectStrongPtr self) {
            if (auto packet = weak_packet.lock()) {
              set_view_matrix(packet);
              set_projection_matrix(packet);
              glDepthFunc(GL_LEQUAL);
              packet->mesh->draw();
              glDepthFunc(GL_LESS);
            }
          });
      }
    });
}

void
TestScene::setup_cube(const SceneObjectStrongPtr& cube)
{
  cube->physics.set_gravity(false);
  cube->local_transform.translate({ 5.f, 2.f, 4.f });
  cube->with_render_packet(
    m_app_cache, [this, cube](RenderPacketWeakPtr weak_packet) {
      if (auto packet = weak_packet.lock()) {
        packet->shader_program->load(m_app_cache.shader_repo,
                                     { "cube.vert.glsl", "cube.frag.glsl" });
        packet->mesh->template load<Vertex_PosColNorm, TriangleIndices>(
          { { 0.5f * glm::vec3(CUBE_VERT0),
              { 0, 0, 0 },
              { -0.577f, -0.577f, -0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT1),
              { 1, 0, 0 },
              { 0.577f, -0.577f, -0.577f } },
            { 0.5f * glm::vec3(CUBE_VERT2),
              { 0.577f, 0.577f, -0.577f },
              { 1, 1, 0 } },
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
              { -0.577f, 0.577f, 0.577f } } },
          { CUBE_FACES },
          GL_TRIANGLES);

        packet->render = SceneObject::capture_weak(
          cube, [this, weak_packet](SceneObjectStrongPtr self) {
            if (auto packet = weak_packet.lock()) {
              set_view_matrix(packet);
              set_projection_matrix(packet);
              set_model_matrix(packet, self->get_world_transformation_mat());
              packet->mesh->draw();
            }
          });
      }
    });

  cube->local_transform.scale({ 2.f, 1.f, 3.f });
  cube->local_transform.rotate(AXIS_Y, glm::radians(45.f));
  cube->local_transform.rotate(AXIS_X, glm::radians(60.f));
  cube->local_transform.rotate(AXIS_Z, glm::radians(30.f));
}

void
TestScene::setup_frustum(const SceneObjectStrongPtr& frustum)
{
  frustum->physics.set_gravity(false);
  auto initial_frustum_vertices =
    m_app_cache.get_camera().get_frustum_worldspace();

  frustum->with_render_packet(
    m_app_cache,
    [this, frustum, initial_frustum_vertices](RenderPacketWeakPtr weak_packet) {
      if (auto packet = weak_packet.lock()) {
        packet->shader_program->load(m_app_cache.shader_repo,
                                     { "AABB.vert.glsl", "AABB.frag.glsl" });
        packet->mesh->template load<Vertex_Pos, LineIndices>(
          initial_frustum_vertices, { CUBE_EDGES }, GL_LINES);

        packet->render = SceneObject::capture_weak(
          frustum, [this, weak_packet](SceneObjectStrongPtr self) {
            if (auto packet = weak_packet.lock()) {
              set_model_matrix(packet, self->get_world_transformation_mat());
              set_view_matrix(packet);
              set_projection_matrix(packet);
              packet->mesh->draw();
            }
          });
      }
    });
}

void
TestScene::setup_objmodel(const SceneObjectStrongPtr& objmodel)
{
  objmodel->physics.set_gravity(false);
  objmodel->with_render_packet(
    m_app_cache, [this, objmodel](RenderPacketWeakPtr weak_packet) {
      if (auto packet = weak_packet.lock()) {
        packet->shader_program->load(m_app_cache.shader_repo,
                                     { "AABB.vert.glsl", "AABB.frag.glsl" });
        packet->mesh->template load<Vertex_Pos, TriangleIndices>("cube.obj",
                                                                 GL_TRIANGLES);

        packet->render = SceneObject::capture_weak(
          objmodel, [this, weak_packet](SceneObjectStrongPtr self) {
            if (auto packet = weak_packet.lock()) {
              set_view_matrix(packet);
              set_projection_matrix(packet);
              set_model_matrix(packet, self->get_world_transformation_mat());
              packet->mesh->draw();
            }
          });
      }
    });
}
