#include <rivalgrounds/RivalGrounds.h>

#ifndef NDEBUG
namespace {
constexpr float DEBUG_SPAWN_OFFSET = 8.0f;
constexpr float DEBUG_PATROL_OFFSET = 6.0f;
constexpr float DEBUG_MIN_FOV = 35.0f;
constexpr float DEBUG_MAX_FOV = 110.0f;
constexpr float DEBUG_MIN_CAMERA_SPEED = 0.25f;
constexpr float DEBUG_MAX_CAMERA_SPEED = 30.0f;
constexpr float DEBUG_MIN_SENSITIVITY = 0.05f;
constexpr float DEBUG_MAX_SENSITIVITY = 2.0f;
}

void
RivalGrounds::render_debug_menu()
{
  if (!m_debug_menu_open)
    return;

  ImGui::SetNextWindowSize(ImVec2(440.0f, 520.0f), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Rival Grounds Debug (F1)", &m_debug_menu_open)) {
    if (ImGui::BeginTabBar("DebugTabs")) {
      if (ImGui::BeginTabItem("Player")) {
        const glm::vec3 position =
          m_player->get_local_transform().get_position();
        ImGui::Checkbox("Noclip (WASD + E/Q)", &m_debug_noclip);
        ImGui::Checkbox("Invincible", &m_debug_invincible);
        ImGui::Checkbox("Show collision hitboxes", &m_debug_show_hitboxes);
        ImGui::Text(
          "Position: %.3f, %.3f, %.3f", position.x, position.y, position.z);
        ImGui::Text(
          "Health: %.1f / %.1f", m_player_health, m_player_max_health);
        ImGui::Text("Grounded: %s", m_player_grounded ? "yes" : "no");
        ImGui::Text("Vertical velocity: %.3f", m_player_vy);
        ImGui::Text("Game state: %d", static_cast<int>(m_game_state));
        static float teleport_xyz[3] = { 0.0f, 0.0f, 0.0f };

        ImGui::Separator();
        ImGui::TextUnformatted("Teleport to coordinates");

        ImGui::InputFloat3("X / Y / Z", teleport_xyz, "%.2f");

        if (ImGui::Button("Use current position")) {
          teleport_xyz[0] = position.x;
          teleport_xyz[1] = position.y;
          teleport_xyz[2] = position.z;
        }

        ImGui::SameLine();

        if (ImGui::Button("Teleport##xyz")) {
          m_player->get_local_transform().set_position(
            glm::vec3(teleport_xyz[0], teleport_xyz[1], teleport_xyz[2]));

          m_player_vy = 0.0f;
          m_player_grounded = false;

          m_app_cache.get_camera().set_view_matrix_dirty();
        }
        if (ImGui::Button("Teleport to spawn")) {
          m_player->get_local_transform().set_position(m_player_spawn);
          m_player_vy = 0.0f;
          m_player_grounded = false;
          m_app_cache.get_camera().set_view_matrix_dirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Spawn enemy nearby")) {
          const glm::vec3 spawn =
            position + glm::vec3(DEBUG_SPAWN_OFFSET, 0.0f, DEBUG_SPAWN_OFFSET);
          const std::vector<glm::vec3> route = {
            spawn + glm::vec3(DEBUG_PATROL_OFFSET, 0.0f, 0.0f),
            spawn - glm::vec3(DEBUG_PATROL_OFFSET, 0.0f, 0.0f)
          };
          m_enemies.push_back(std::make_shared<Enemy>(*this, spawn, route));
        }
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Camera")) {
        Camera& camera = m_app_cache.get_camera();
        const glm::vec3 forward = camera.get_forward();
        ImGui::Text(
          "Forward: %.3f, %.3f, %.3f", forward.x, forward.y, forward.z);
        ImGui::Text("FOV: %.2f deg", camera.get_fovy());
        ImGui::Text("Move speed: %.2f", camera.get_speed());
        ImGui::Text("Mouse sensitivity: %.3f", camera.get_sensitivity());
        ImGui::Text("Near / far: %.3f / %.1f",
                    camera.get_near_plane(),
                    camera.get_far_plane());
        float fov = camera.get_fovy();
        if (ImGui::SliderFloat("FOV", &fov, DEBUG_MIN_FOV, DEBUG_MAX_FOV))
          camera.set_fovy(fov);
        float speed = camera.get_speed();
        if (ImGui::SliderFloat("Move speed",
                               &speed,
                               DEBUG_MIN_CAMERA_SPEED,
                               DEBUG_MAX_CAMERA_SPEED))
          camera.set_speed(speed);
        float sensitivity = camera.get_sensitivity();
        if (ImGui::SliderFloat("Sensitivity",
                               &sensitivity,
                               DEBUG_MIN_SENSITIVITY,
                               DEBUG_MAX_SENSITIVITY))
          camera.set_sensitivity(sensitivity);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("World / Physics")) {
        ImGui::Text("Static collision meshes: %zu",
                    m_collision_world.static_mesh_count());
        ImGui::Text("Physics objects: %zu", get_all_physics_objects().size());
        ImGui::Text("Fixed simulation step: %.5f s", FIXED_STEP);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Enemies")) {
        std::size_t patrol_count = 0;
        std::size_t combat_count = 0;
        std::size_t dead_count = 0;
        for (const auto& enemy : m_enemies) {
          if (!enemy)
            continue;
          switch (enemy->get_state()) {
            case EnemyState::PATROL:
              ++patrol_count;
              break;
            case EnemyState::COMBAT:
              ++combat_count;
              break;
            case EnemyState::DEAD:
              ++dead_count;
              break;
          }
        }
        ImGui::Text("Total: %zu", m_enemies.size());
        ImGui::Text("Patrol: %zu  Combat: %zu  Dead: %zu",
                    patrol_count,
                    combat_count,
                    dead_count);
        if (ImGui::BeginTable("EnemyList",
                              5,
                              ImGuiTableFlags_Borders |
                                ImGuiTableFlags_RowBg)) {
          ImGui::TableSetupColumn("#");
          ImGui::TableSetupColumn("State");
          ImGui::TableSetupColumn("Position");
          ImGui::TableSetupColumn("Health");
          ImGui::TableSetupColumn("Flash");
          ImGui::TableHeadersRow();
          for (std::size_t index = 0; index < m_enemies.size(); ++index) {
            const auto& enemy = m_enemies[index];
            if (!enemy)
              continue;
            const glm::vec3 enemy_position = enemy->get_position();
            const char* state =
              enemy->get_state() == EnemyState::PATROL   ? "Patrol"
              : enemy->get_state() == EnemyState::COMBAT ? "Combat"
                                                         : "Dead";
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%zu", index);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(state);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.1f %.1f %.1f",
                        enemy_position.x,
                        enemy_position.y,
                        enemy_position.z);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text(
              "%.0f / %.0f", enemy->get_health(), enemy->get_max_health());
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%.2f", enemy->get_damage_flash());
          }
          ImGui::EndTable();
        }
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::TextUnformatted(
      "F1 toggles this menu. Close it to recapture the mouse.");
  }
  ImGui::End();

  if (!m_debug_menu_open)
    m_app_cache.get_window().set_cursor_mode(GLFW_CURSOR_DISABLED);
}
#endif
