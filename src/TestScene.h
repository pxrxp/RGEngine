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

    // Start player standing in front of the citadel
    if (m_player) {
      m_player->get_local_transform().set_position(
        { 0.0f, PLAYER_EYE_HEIGHT, 14.0f });
      m_app_cache.get_camera().look_at({ 0.0f, PLAYER_EYE_HEIGHT, 0.0f });
    }

    // 1. Skybox
    m_skybox = std::make_shared<SceneObject>();
    setup_skybox(m_skybox, "skybox");
    m_scene_ptr->add_child(m_skybox);

    // 2. Ground
    auto plane = std::make_shared<SceneObject>();
    setup_plane(plane);
    m_scene_ptr->add_child(plane);

    // Palette
    const glm::vec3 c_stone = { 0.76f, 0.72f, 0.65f };
    const glm::vec3 c_stone_dark = { 0.48f, 0.45f, 0.42f };
    const glm::vec3 c_gold = { 0.95f, 0.75f, 0.25f };
    const glm::vec3 c_wood = { 0.55f, 0.35f, 0.20f };
    const glm::vec3 c_leaves = { 0.25f, 0.55f, 0.25f };
    const glm::vec3 c_crate = { 0.70f, 0.45f, 0.25f };

    // 3. Central Monument & Relic
    add_box({ 0.f, 0.25f, 0.f }, { 6.f, 0.5f, 6.f }, c_stone_dark);
    add_box({ 0.f, 0.75f, 0.f }, { 4.f, 0.5f, 4.f }, c_stone);
    add_box({ 0.f, 2.5f, 0.f }, { 0.8f, 3.0f, 0.8f }, c_gold);

    // 4. Grand Pillars & Lintels
    for (float x : { -6.0f, 6.0f }) {
      for (float z = -6.0f; z <= 6.0f; z += 3.0f) {
        build_pillar({ x, 0.0f, z }, 5.0f, c_stone, c_stone_dark);
      }
      add_box({ x, 5.25f, 0.0f }, { 1.2f, 0.5f, 13.5f }, c_stone);
    }

    for (float x = -3.0f; x <= 3.0f; x += 3.0f) {
      build_pillar({ x, 0.0f, -6.0f }, 5.0f, c_stone, c_stone_dark);
    }
    add_box({ 0.0f, 5.25f, -6.0f }, { 13.5f, 0.5f, 1.2f }, c_stone);

    // 5. Grand Walkable Staircase
    for (int i = 0; i < 8; ++i) {
      float z = -8.0f - i * 1.0f;
      float h = 0.4f * (i + 1);
      add_box({ 0.0f, h * 0.5f, z }, { 5.0f, h, 1.0f }, c_stone_dark);
    }

    // High Lookout Platform
    add_box({ 0.0f, 1.6f, -18.5f }, { 10.0f, 3.2f, 6.0f }, c_stone);

    // 6. Voxel Trees
    build_tree({ -10.0f, 0.0f, 4.0f }, c_wood, c_leaves);
    build_tree({ -12.0f, 0.0f, -2.0f }, c_wood, c_leaves);
    build_tree({ 10.0f, 0.0f, 3.0f }, c_wood, c_leaves);
    build_tree({ 11.0f, 0.0f, -4.0f }, c_wood, c_leaves);

    // 7. Interactive Physics Crates
    spawn_physics_box({ -3.0f, 6.0f, 5.0f }, { 1.0f, 1.0f, 1.0f }, c_crate);
    spawn_physics_box({ -3.1f, 8.5f, 5.1f }, { 0.9f, 0.9f, 0.9f }, c_crate);
  }

private:
  void build_pillar(const glm::vec3& pos,
                    float height,
                    const glm::vec3& shaft_col,
                    const glm::vec3& cap_col)
  {
    add_box({ pos.x, 0.25f, pos.z }, { 1.2f, 0.5f, 1.2f }, cap_col);
    add_box({ pos.x, 0.5f + height * 0.5f, pos.z },
            { 0.8f, height, 0.8f },
            shaft_col);
    add_box(
      { pos.x, 0.5f + height + 0.25f, pos.z }, { 1.2f, 0.5f, 1.2f }, cap_col);
  }

  void build_tree(const glm::vec3& pos,
                  const glm::vec3& trunk_col,
                  const glm::vec3& leaves_col)
  {
    add_box({ pos.x, 2.0f, pos.z }, { 0.8f, 4.0f, 0.8f }, trunk_col);
    add_box({ pos.x, 4.5f, pos.z }, { 3.5f, 1.5f, 3.5f }, leaves_col);
    add_box({ pos.x, 5.8f, pos.z }, { 2.5f, 1.5f, 2.5f }, leaves_col * 1.1f);
    add_box({ pos.x, 6.8f, pos.z }, { 1.5f, 1.0f, 1.5f }, leaves_col * 1.2f);
  }

  MeshDescriptor<Vertex_PosColNorm, TriangleIndices> get_sharp_box_descriptor()
  {
    std::vector<Vertex_PosColNorm> vertices;
    std::vector<TriangleIndices> indices;

    auto add_face =
      [&](glm::vec3 n, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
        GLuint base = static_cast<GLuint>(vertices.size());
        vertices.push_back({ p0, { 1.f, 1.f, 1.f }, n });
        vertices.push_back({ p1, { 1.f, 1.f, 1.f }, n });
        vertices.push_back({ p2, { 1.f, 1.f, 1.f }, n });
        vertices.push_back({ p3, { 1.f, 1.f, 1.f }, n });
        indices.push_back({ base, base + 1, base + 2 });
        indices.push_back({ base, base + 2, base + 3 });
      };

    add_face({ 0, 1, 0 },
             { -0.5, 0.5, -0.5 },
             { -0.5, 0.5, 0.5 },
             { 0.5, 0.5, 0.5 },
             { 0.5, 0.5, -0.5 });
    add_face({ 0, -1, 0 },
             { -0.5, -0.5, 0.5 },
             { -0.5, -0.5, -0.5 },
             { 0.5, -0.5, -0.5 },
             { 0.5, -0.5, 0.5 });
    add_face({ 0, 0, 1 },
             { -0.5, -0.5, 0.5 },
             { 0.5, -0.5, 0.5 },
             { 0.5, 0.5, 0.5 },
             { -0.5, 0.5, 0.5 });
    add_face({ 0, 0, -1 },
             { 0.5, -0.5, -0.5 },
             { -0.5, -0.5, -0.5 },
             { -0.5, 0.5, -0.5 },
             { 0.5, 0.5, -0.5 });
    add_face({ -1, 0, 0 },
             { -0.5, -0.5, -0.5 },
             { -0.5, -0.5, 0.5 },
             { -0.5, 0.5, 0.5 },
             { -0.5, 0.5, -0.5 });
    add_face({ 1, 0, 0 },
             { 0.5, -0.5, 0.5 },
             { 0.5, -0.5, -0.5 },
             { 0.5, 0.5, -0.5 },
             { 0.5, 0.5, 0.5 });

    return {
      "sharp_box_mesh", std::move(vertices), std::move(indices), GL_TRIANGLES
    };
  }

  void add_box(const glm::vec3& center,
               const glm::vec3& size,
               const glm::vec3& color)
  {
    auto obj = std::make_shared<SceneObject>();
    obj->get_local_transform().set_position(center).set_scale(size);
    obj->get_physics_component().disable();

    std::string mat_name = "mat_" + std::to_string(color.r) + "_" +
                           std::to_string(color.g) + "_" +
                           std::to_string(color.b);

    obj->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(get_sharp_box_descriptor()),
      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = mat_name,
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = {},
        .uniforms = { { "u_color", color } } }),
      capture_weak(
        obj, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        }));

    m_scene_ptr->add_child(obj);
  }

  void spawn_physics_box(const glm::vec3& center,
                         const glm::vec3& size,
                         const glm::vec3& color)
  {
    auto obj = std::make_shared<SceneObject>();
    obj->get_local_transform().set_position(center).set_scale(size);
    obj->get_physics_component().set_mass(1.0f).set_gravity(true);

    std::string mat_name = "dyn_mat_" + std::to_string(color.r) + "_" +
                           std::to_string(color.g) + "_" +
                           std::to_string(color.b);

    obj->set_render_packet(
      m_app_cache.mesh_repo.load_mesh(get_sharp_box_descriptor()),
      m_app_cache.material_repo.load_material(MaterialDescriptor{
        .material_name = mat_name,
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = {},
        .uniforms = { { "u_color", color } } }),
      capture_weak(
        obj, [this](SceneObjectStrongPtr self, RenderPacketStrongPtr packet) {
          set_view_matrix(packet);
          set_projection_matrix(packet);
          set_model_matrix(packet, self->get_world_transformation_mat());
          packet->mesh->draw();
        }));

    m_scene_ptr->add_child(obj);
  }

  void setup_plane(const SceneObjectStrongPtr& plane)
  {
    const auto far = m_app_cache.get_camera().get_far_plane();
    plane->get_local_transform()
      .set_position({ 0.0f, 0.0f, 0.0f })
      .scale({ far, far, far })
      .rotate_deg(AXIS_X, -90.f);
    plane->get_physics_component().disable();

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
};
