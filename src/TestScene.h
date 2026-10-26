#pragma once
#include "App.h"
#include "Enemy.h"
#include "Material.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "ShaderProgram.h"
#include "components/vertex_formats.h"
#include "repos/MaterialRepo.h"
#include "repos/MeshRepo.h"
#include "util/opengl.h"
#include <imgui.h>

enum class MatchState
{
  PLAYING,
  VICTORY,
  DEFEATED
};

class TestScene : public Scene
{
private:
  std::shared_ptr<Enemy> m_enemy;
  MatchState m_match_state = MatchState::PLAYING;
  float m_flash_red_timer = 0.0f;

  const glm::vec3 m_player_spawn = { 0.0f, PLAYER_EYE_HEIGHT, 18.0f };
  const glm::vec3 m_enemy_spawn = { 0.0f, 0.0f, -14.0f };

public:
  TestScene(App& app)
    : Scene(app)
  {
  }

  void init() override
  {
    Scene::init();
    debug_camera();

    reset_match();
  }

  void reset_match()
  {
    m_match_state = MatchState::PLAYING;

    if (m_player) {
      m_player->get_local_transform().set_position(m_player_spawn);
      m_app_cache.get_camera().look_at({ 0.0f, PLAYER_EYE_HEIGHT, 0.0f });
    }

    m_scene_ptr = std::make_shared<SceneObject>();
    if (m_player)
      m_scene_ptr->add_child(m_player);

    m_skybox = std::make_shared<SceneObject>();
    setup_skybox(m_skybox, "skybox");
    m_scene_ptr->add_child(m_skybox);

    auto plane = std::make_shared<SceneObject>();
    setup_plane(plane);
    m_scene_ptr->add_child(plane);

    build_parkour_compound();

    std::vector<glm::vec3> patrol_route = { { -10.0f, 0.0f, -12.0f },
                                            { 10.0f, 0.0f, -12.0f },
                                            { 10.0f, 0.0f, 4.0f },
                                            { -10.0f, 0.0f, 4.0f } };
    m_enemy = std::make_shared<Enemy>(*this, m_enemy_spawn, patrol_route);
  }

  void update_physics() override
  {
    Scene::update_physics();

    float dt = Scene::FIXED_STEP;

    if (m_app_cache.get_window().is_key_pressed(GLFW_KEY_R)) {
      if (m_match_state != MatchState::PLAYING) {
        reset_match();
        return;
      }
    }

    if (m_match_state != MatchState::PLAYING || !m_enemy)
      return;

    m_enemy->update(*this, dt);

    if (m_flash_red_timer > 0.0f)
      m_flash_red_timer -= dt;

    if (m_enemy->get_state() == EnemyState::HUNT) {
      glm::vec3 ep = m_enemy->get_position();
      glm::vec3 pp = m_player->get_local_transform().get_position();
      float h_dist =
        glm::distance(glm::vec2(ep.x, ep.z), glm::vec2(pp.x, pp.z));
      if (h_dist < 1.35f) {
        m_match_state = MatchState::DEFEATED;
      }
    }
  }

  void submit_to_renderer()
  {
    Scene::submit_to_renderer();
    render_game_hud();
  }

protected:
  void handle_shooting() override
  {
    auto& window = m_app_cache.get_window();
    bool is_pressed = window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);

    if (is_pressed && !m_mouse_was_pressed &&
        m_match_state == MatchState::PLAYING) {
      glm::vec3 ray_origin = m_player->get_local_transform().get_position();
      glm::vec3 ray_dir = m_app_cache.get_camera().get_forward();

      // Enemy hit
      if (m_enemy && !m_enemy->is_dead()) {
        float t = 0.0f;
        glm::vec3 hit_norm(0.0f);
        if (m_enemy->get_hitbox().intersects_ray(
              ray_origin, ray_dir, t, hit_norm)) {
          m_flash_red_timer = 0.12f;
          bool died = m_enemy->take_damage(35.0f, ray_dir * 1.5f);
          if (died) {
            m_match_state = MatchState::VICTORY;
          }
        }
      }

      // Physics box reaction
      auto objects = get_all_physics_objects();
      for (auto& obj : objects) {
        float t = 0.0f;
        glm::vec3 normal(0.0f);
        if (obj->get_world_AABB().intersects_ray(
              ray_origin, ray_dir, t, normal)) {
          glm::vec3 hit_point = ray_origin + ray_dir * t;
          obj->get_physics_component().apply_impulse_at_point(
            ray_dir * 25.0f + glm::vec3(0.0f, 4.0f, 0.0f), hit_point);
        }
      }
    }

    m_mouse_was_pressed = is_pressed;
  }

private:
  void render_game_hud()
  {
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    const ImGuiIO& io = ImGui::GetIO();
    float cx = io.DisplaySize.x * 0.5f;

    // Subtle hit tick
    if (m_flash_red_timer > 0.0f) {
      float cy = io.DisplaySize.y * 0.5f;
      draw->AddLine(ImVec2(cx - 5, cy - 5),
                    ImVec2(cx + 5, cy + 5),
                    IM_COL32(235, 75, 75, 255),
                    1.8f);
      draw->AddLine(ImVec2(cx + 5, cy - 5),
                    ImVec2(cx - 5, cy + 5),
                    IM_COL32(235, 75, 75, 255),
                    1.8f);
    }

    if (m_match_state == MatchState::PLAYING && m_enemy) {
      const char* status = "Patrolling";
      ImU32 status_col = IM_COL32(140, 200, 140, 220);

      if (m_enemy->get_state() == EnemyState::INVESTIGATE) {
        status = "Investigating noise";
        status_col = IM_COL32(235, 195, 80, 220);
      } else if (m_enemy->get_state() == EnemyState::HUNT) {
        status = "Hunting you";
        status_col = IM_COL32(240, 75, 75, 240);
      }

      ImVec2 sz = ImGui::CalcTextSize(status);
      draw->AddText(ImVec2(cx - sz.x * 0.5f, 24), status_col, status);

      // Enemy health bar
      float hp = m_enemy->get_health() / 100.0f;
      float w = 180.0f;
      draw->AddRectFilled(ImVec2(cx - w * 0.5f, 46),
                          ImVec2(cx + w * 0.5f, 51),
                          IM_COL32(35, 35, 40, 200));
      draw->AddRectFilled(ImVec2(cx - w * 0.5f, 46),
                          ImVec2(cx - w * 0.5f + w * hp, 51),
                          IM_COL32(215, 60, 60, 230));

    } else if (m_match_state == MatchState::VICTORY) {
      const char* text = "Target eliminated";
      const char* sub = "Press R to play again";
      ImVec2 tsz = ImGui::CalcTextSize(text);
      ImVec2 ssz = ImGui::CalcTextSize(sub);

      draw->AddText(
        ImVec2(cx - tsz.x * 0.5f, 130), IM_COL32(110, 235, 130, 240), text);
      draw->AddText(
        ImVec2(cx - ssz.x * 0.5f, 155), IM_COL32(180, 180, 180, 200), sub);

    } else if (m_match_state == MatchState::DEFEATED) {
      const char* text = "You died";
      const char* sub = "Press R to retry";
      ImVec2 tsz = ImGui::CalcTextSize(text);
      ImVec2 ssz = ImGui::CalcTextSize(sub);

      draw->AddText(
        ImVec2(cx - tsz.x * 0.5f, 130), IM_COL32(235, 75, 75, 240), text);
      draw->AddText(
        ImVec2(cx - ssz.x * 0.5f, 155), IM_COL32(180, 180, 180, 200), sub);
    }
  }

  void build_parkour_compound()
  {
    // PUBG/Doom style colors
    const glm::vec3 c_container_blue = { 0.22f, 0.38f, 0.52f };
    const glm::vec3 c_container_red = { 0.58f, 0.22f, 0.20f };
    const glm::vec3 c_concrete = { 0.50f, 0.48f, 0.46f };
    const glm::vec3 c_metal_catwalk = { 0.32f, 0.34f, 0.36f };
    const glm::vec3 c_crate = { 0.62f, 0.44f, 0.26f };

    // Perimeter Arena Walls
    add_box({ 0.0f, 3.0f, -24.0f }, { 48.0f, 6.0f, 1.0f }, c_concrete);
    add_box({ -24.0f, 3.0f, 0.0f }, { 1.0f, 6.0f, 48.0f }, c_concrete);
    add_box({ 24.0f, 3.0f, 0.0f }, { 1.0f, 6.0f, 48.0f }, c_concrete);
    add_box({ 0.0f, 3.0f, 24.0f }, { 48.0f, 6.0f, 1.0f }, c_concrete);

    // 1. Shipping Containers (Ground & Stacked)
    // Blue Container on ground (height = 2.6m, width = 2.4m, length = 6.0m)
    add_box({ -8.0f, 1.3f, -4.0f }, { 2.4f, 2.6f, 6.0f }, c_container_blue);
    // Stacked Red Container on top (accessible by parkour!)
    add_box({ -8.0f, 3.9f, -4.0f }, { 2.4f, 2.6f, 6.0f }, c_container_red);

    // Right Side Shipping Containers
    add_box({ 8.0f, 1.3f, -4.0f }, { 2.4f, 2.6f, 6.0f }, c_container_red);
    add_box({ 8.0f, 1.3f, 4.0f }, { 2.4f, 2.6f, 6.0f }, c_container_blue);

    // 2. Parkour Route 1 (Stepping boxes up to container roof)
    add_box({ -8.0f, 0.35f, 0.5f }, { 1.2f, 0.7f, 1.2f }, c_concrete);
    add_box({ -8.0f, 0.95f, 2.0f }, { 1.2f, 1.2f, 1.2f }, c_concrete);
    add_box({ -8.0f, 1.65f, 3.5f }, { 1.2f, 1.4f, 1.2f }, c_concrete);

    // 3. Central Catwalk / Sky Bridge (Connects East & West rooftop positions)
    add_box({ 0.0f, 2.6f, -4.0f }, { 13.6f, 0.3f, 1.8f }, c_metal_catwalk);
    // Central sniper perch on catwalk
    add_box({ 0.0f, 3.2f, -4.0f }, { 2.0f, 0.9f, 2.0f }, c_metal_catwalk);

    // 4. North Bunker & Rooftop Terrace
    add_box({ -6.0f, 2.0f, -14.0f }, { 1.0f, 4.0f, 8.0f }, c_concrete);
    add_box({ 6.0f, 2.0f, -14.0f }, { 1.0f, 4.0f, 8.0f }, c_concrete);
    add_box({ 0.0f, 2.0f, -18.0f }, { 12.0f, 4.0f, 1.0f }, c_concrete);
    add_box({ 0.0f, 4.15f, -14.0f }, { 13.0f, 0.3f, 8.0f }, c_concrete);

    // Parkour Stairs up to Bunker Rooftop
    for (int i = 0; i < 8; ++i) {
      float z = -9.0f - i * 1.0f;
      float h = 0.5f * (i + 1);
      add_box({ 7.2f, h * 0.5f, z }, { 1.6f, h, 1.0f }, c_concrete);
    }

    // 5. Parkour Gap Jumps (jump across crates from container to terrace)
    add_box({ 3.5f, 2.4f, -9.0f }, { 1.4f, 0.6f, 1.4f }, c_metal_catwalk);
    add_box({ -3.5f, 2.4f, -9.0f }, { 1.4f, 0.6f, 1.4f }, c_metal_catwalk);

    // 6. Dynamic Physics Crates for tactical cover & shooting
    spawn_physics_box({ 0.0f, 0.6f, 4.0f }, { 1.2f, 1.2f, 1.2f }, c_crate);
    spawn_physics_box({ 0.2f, 1.8f, 4.0f }, { 1.0f, 1.0f, 1.0f }, c_crate);
    spawn_physics_box({ -4.0f, 0.6f, -4.0f }, { 1.1f, 1.1f, 1.1f }, c_crate);
    spawn_physics_box({ 4.0f, 0.6f, -4.0f }, { 1.1f, 1.1f, 1.1f }, c_crate);
    spawn_physics_box({ -2.0f, 0.6f, 10.0f }, { 1.2f, 1.2f, 1.2f }, c_crate);
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
    obj->get_physics_component().set_mass(1.2f).set_gravity(true);

    std::string mat_name =
      "dyn_mat_" + std::to_string(color.r) + "_" + std::to_string(color.g);

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
