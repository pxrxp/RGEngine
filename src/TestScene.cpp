#include "TestScene.h"
#include "Mesh.h"
#include "MeshRepo.h"
#include "components/PhysicsComponent.h"
#include "components/TransformComponent.h"
#include "components/vertex_formats.h"
#include "util/definitions.h"

void
TestScene::init()
{
  Scene::init();

  auto skybox = std::make_shared<SceneObject>();
  setup_skybox(skybox);
  m_scene_ptr->add_child(skybox);

  // auto cube = std::make_shared<SceneObject>();
  // setup_cube(cube);
  // m_scene_ptr->add_child(cube);

  // auto frustum = std::make_shared<SceneObject>();
  // setup_frustum(frustum);
  // m_scene_ptr->add_child(frustum);

  // auto objmodel = std::make_shared<SceneObject>();
  // setup_objmodel(objmodel);
  // m_scene_ptr->add_child(objmodel);
}

void
TestScene::setup_skybox(const SceneObjectStrongPtr& skybox)
{
  std::string id = "skybox";
  skybox->physics.set_gravity(false);
  skybox->set_render_packet(
    m_app_cache.mesh_repo.load_mesh(MeshDescriptor<Vertex_Pos, TriangleIndices>{
      .mesh_name = id,
      .vertices = { CUBE_VERTICES },
      .indices = { CUBE_FACES },
      .draw_primitive = GL_TRIANGLES }),

    m_app_cache.material_repo.load_material(MaterialDescriptor{
      .material_name = id,
      .shader_program_desc = { .program_name = id,
                               .shaders = { "skybox.vert", "skybox.frag" } },
      .texture_desc_list = { { .texture_name = id,
                               .target = GL_TEXTURE_CUBE_MAP } } }),

    capture_weak(skybox,
                 [this]([[maybe_unused]] SceneObjectStrongPtr self,
                        RenderPacketStrongPtr packet) {
                   set_view_matrix(packet);
                   set_projection_matrix(packet);
                   glDepthFunc(GL_LEQUAL);
                   packet->mesh->draw();
                   glDepthFunc(GL_LESS);
                 }));
}

// void
// TestScene::setup_cube(const SceneObjectStrongPtr& cube)
// {
//   cube->physics.set_gravity(false);
//   cube->local_transform.translate({ 5.f, 2.f, 4.f });
//   cube->with_render_packet(
//     m_app_cache, [this, cube](RenderPacketWeakPtr weak_packet) {
//       if (auto packet = weak_packet.lock()) {
//         packet->shader_program->load(m_app_cache.shader_repo,
//                                      { "cube.vert.glsl", "cube.frag.glsl" });
//         packet->mesh->template load<Vertex_PosColNorm, TriangleIndices>(
//           { { 0.5f * glm::vec3(CUBE_VERT0),
//               { 0, 0, 0 },
//               { -0.577f, -0.577f, -0.577f } },
//             { 0.5f * glm::vec3(CUBE_VERT1),
//               { 1, 0, 0 },
//               { 0.577f, -0.577f, -0.577f } },
//             { 0.5f * glm::vec3(CUBE_VERT2),
//               { 0.577f, 0.577f, -0.577f },
//               { 1, 1, 0 } },
//             { 0.5f * glm::vec3(CUBE_VERT3),
//               { 0, 1, 0 },
//               { -0.577f, 0.577f, -0.577f } },
//             { 0.5f * glm::vec3(CUBE_VERT4),
//               { 0, 0, 1 },
//               { -0.577f, -0.577f, 0.577f } },
//             { 0.5f * glm::vec3(CUBE_VERT5),
//               { 1, 0, 1 },
//               { 0.577f, -0.577f, 0.577f } },
//             { 0.5f * glm::vec3(CUBE_VERT6),
//               { 1, 1, 1 },
//               { 0.577f, 0.577f, 0.577f } },
//             { 0.5f * glm::vec3(CUBE_VERT7),
//               { 0, 1, 1 },
//               { -0.577f, 0.577f, 0.577f } } },
//           { CUBE_FACES },
//           GL_TRIANGLES);

//         packet->render = SceneObject::capture_weak(
//           cube, [this, weak_packet](SceneObjectStrongPtr self) {
//             if (auto packet = weak_packet.lock()) {
//               set_view_matrix(packet);
//               set_projection_matrix(packet);
//               set_model_matrix(packet, self->get_world_transformation_mat());
//               packet->mesh->draw();
//             }
//           });
//       }
//     });

//   cube->local_transform.scale({ 2.f, 1.f, 3.f });
//   cube->local_transform.rotate(AXIS_Y, glm::radians(45.f));
//   cube->local_transform.rotate(AXIS_X, glm::radians(60.f));
//   cube->local_transform.rotate(AXIS_Z, glm::radians(30.f));
// }

// void
// TestScene::setup_frustum(const SceneObjectStrongPtr& frustum)
// {
//   frustum->physics.set_gravity(false);
//   auto initial_frustum_vertices =
//     m_app_cache.get_camera().get_frustum_worldspace();

//   render_packet->shader_program->load(m_app_cache.shader_repo,
//                                       { "AABB.vert.glsl", "AABB.frag.glsl"
//                                       });
//   render_packet->mesh->template load<Vertex_Pos, LineIndices>(
//     initial_frustum_vertices, { CUBE_EDGES }, GL_LINES);

//   render_packet->render =
//     capture_weak(frustum, render_packet, [this](auto self, auto packet))
//   {

//     set_model_matrix(packet, self->get_world_transformation_mat());
//     set_view_matrix(packet);
//     set_projection_matrix(packet);
//     packet->mesh->draw();
//   };
// }

// void
// TestScene::setup_objmodel(const SceneObjectStrongPtr& objmodel)
// {
//   objmodel->physics.set_gravity(false);
//   objmodel->with_render_packet(
//     m_app_cache, [this, objmodel](RenderPacketWeakPtr weak_packet) {
//       if (auto packet = weak_packet.lock()) {
//         packet->shader_program->load(m_app_cache.shader_repo,
//                                      { "AABB.vert.glsl", "AABB.frag.glsl" });
//         packet->mesh->template load<Vertex_Pos, TriangleIndices>("cube.obj",
//                                                                  GL_TRIANGLES);

//         packet->render = SceneObject::capture_weak(
//           objmodel, [this, weak_packet](SceneObjectStrongPtr self) {
//             if (auto packet = weak_packet.lock()) {
//               set_view_matrix(packet);
//               set_projection_matrix(packet);
//               set_model_matrix(packet, self->get_world_transformation_mat());
//               packet->mesh->draw();
//             }
//           });
//       }
//     });
// }
