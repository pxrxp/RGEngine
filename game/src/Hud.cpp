#include <rivalgrounds/RivalGrounds.h>

#include <engine/repos/MeshRepo.h>
#include <engine/util/obj_model.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <random>

void
RivalGrounds::render_game_hud()
{
#ifndef NDEBUG
  render_debug_menu();
#endif
  ImDrawList* draw = ImGui::GetForegroundDrawList();
  const ImGuiIO& io = ImGui::GetIO();
  float cx = io.DisplaySize.x * 0.5f;
  float cy = io.DisplaySize.y * 0.5f;

  if (m_game_state == GameState::INTRO) {
    draw->AddRectFilled(
      ImVec2(0, 0), io.DisplaySize, IM_COL32(10, 10, 12, 255));

    float top_y = io.DisplaySize.y * 0.08f;
    if (m_logo_texture) {
      float logo_w = std::min(io.DisplaySize.x * 0.30f, 320.0f);
      float logo_h = logo_w * m_logo_texture->get_aspect_ratio();

      draw->AddImage((ImTextureID)(intptr_t)m_logo_texture->get_id(),
                     ImVec2(cx - logo_w * 0.5f, top_y),
                     ImVec2(cx + logo_w * 0.5f, top_y + logo_h));
      top_y += logo_h + 28.0f;
    }

    const char* backstory =
      "We came down hard. The ship broke up over the rooftops, and what's left "
      "of it is scattered through the streets with the cargo.\n\n"
      "The city was already empty when we landed. The people are still here, "
      "though. Pale as chalk, thin as paper, and they don't want us. They "
      "shoot "
      "before they ask.\n\n"
      "The scroll went out with the cargo on the way down. A roof, a "
      "stairwell, "
      "under a crate. Nobody knows.\n\n"
      "Use what the ship dropped. The crates stack. Stack them, climb, and "
      "bring "
      "the scroll home.";

    const float story_width = std::min(io.DisplaySize.x * 0.55f, 600.0f);
    const float story_font_size = 15.0f;
    const ImVec2 story_size =
      ImGui::CalcTextSize(backstory, nullptr, false, story_width);
    const float story_height =
      story_size.y * (story_font_size / ImGui::GetFontSize());

    draw->AddText(nullptr,
                  story_font_size,
                  ImVec2(cx - story_width * 0.5f, top_y),
                  IM_COL32(178, 183, 192, 235),
                  backstory,
                  nullptr,
                  story_width);
    top_y += story_height + 54.0f;

    const char* controls[] = {
      "WASD  Move",  "Ctrl  Sprint",     "Shift  Sneak",
      "Space  Jump", "Left Click  Fire", "Right Click  Grab and place crate"
    };

    const float column_width = 214.0f;
    const float block_left = cx - column_width * 1.5f;
    for (int i = 0; i < 6; ++i) {
      draw->AddText(
        nullptr,
        15.0f,
        ImVec2(block_left + (i % 3) * column_width, top_y + (i / 3) * 22.0f),
        IM_COL32(165, 170, 180, 220),
        controls[i]);
    }
    top_y += 2 * 22.0f + 32.0f;

    const char* prompt = "Press Space or Click to Begin";
    ImVec2 p_sz = ImGui::CalcTextSize(prompt);
    draw->AddText(nullptr,
                  16.0f,
                  ImVec2(cx - p_sz.x * 0.5f, top_y),
                  IM_COL32(230, 215, 125, 240),
                  prompt);
    return;
  }

  glm::mat4 V = m_app_cache.get_camera().get_view_matrix();
  glm::mat4 P = m_app_cache.get_camera().get_projection_matrix();
  glm::mat4 VP = P * V;

  glm::vec3 player_pos = m_player->get_local_transform().get_position();

  std::vector<BoundingBox> colliders;
  bool needs_enemy_occlusion = false;
  for (const auto& enemy : m_enemies) {
    if (enemy && !enemy->is_dead() &&
        enemy->get_state() == EnemyState::COMBAT &&
        glm::distance(enemy->get_position(), player_pos) <= 35.0f) {
      needs_enemy_occlusion = true;
      break;
    }
  }
  if (needs_enemy_occlusion)
    collect_game_colliders(colliders);
#ifndef NDEBUG
  if (m_debug_show_hitboxes) {
    std::vector<BoundingBox> debug_boxes;
    debug_boxes.reserve(2048);

    // Regular collidable scene objects.
    collect_colliders_recursive(m_scene_ptr, debug_boxes);

    // Enemy hitboxes.
    for (const auto& enemy : m_enemies) {
      if (enemy && !enemy->is_dead())
        debug_boxes.push_back(enemy->get_hitbox());
    }

    const int edges[12][2] = { { 0, 1 }, { 0, 2 }, { 1, 3 }, { 2, 3 },
                               { 4, 5 }, { 4, 6 }, { 5, 7 }, { 6, 7 },
                               { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };

    auto project = [&](const glm::vec3& world, ImVec2& out) {
      const glm::vec4 clip = VP * glm::vec4(world, 1.0f);

      if (clip.w <= 0.05f)
        return false;

      const glm::vec3 ndc = glm::vec3(clip) / clip.w;

      if (ndc.x < -1.5f || ndc.x > 1.5f || ndc.y < -1.5f || ndc.y > 1.5f)
        return false;

      out = ImVec2((ndc.x * 0.5f + 0.5f) * io.DisplaySize.x,
                   (1.0f - (ndc.y * 0.5f + 0.5f)) * io.DisplaySize.y);

      return true;
    };

    ImDrawList* hitbox_draw = ImGui::GetForegroundDrawList();

    std::size_t drawn_boxes = 0;

    for (const BoundingBox& box : debug_boxes) {
      const glm::vec3 mn = box.get_min();
      const glm::vec3 mx = box.get_max();

      const glm::vec3 corners[8] = {
        { mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z }, { mn.x, mx.y, mn.z },
        { mx.x, mx.y, mn.z }, { mn.x, mn.y, mx.z }, { mx.x, mn.y, mx.z },
        { mn.x, mx.y, mx.z }, { mx.x, mx.y, mx.z }
      };

      ImVec2 points[8];
      bool valid[8];

      for (int i = 0; i < 8; ++i)
        valid[i] = project(corners[i], points[i]);

      bool visible = false;

      for (const auto& edge : edges) {
        if (valid[edge[0]] && valid[edge[1]]) {
          hitbox_draw->AddLine(
            points[edge[0]], points[edge[1]], IM_COL32(255, 60, 60, 230), 1.5f);
          visible = true;
        }
      }

      if (visible)
        ++drawn_boxes;
    }

    char label[128];
    std::snprintf(label,
                  sizeof(label),
                  "Collision hitboxes: %zu (visible: %zu)",
                  debug_boxes.size(),
                  drawn_boxes);

    hitbox_draw->AddText(
      ImVec2(16.0f, 40.0f), IM_COL32(255, 80, 80, 255), label);
  }
#endif

  for (const auto& enemy : m_enemies) {
    if (enemy->is_dead() || enemy->get_state() != EnemyState::COMBAT)
      continue;

    glm::vec3 eye_pos = enemy->get_position() + glm::vec3(0.0f, 1.9f, 0.0f);
    glm::vec3 to_player = player_pos - eye_pos;
    float dist_to_player = glm::length(to_player);
    if (dist_to_player > 35.0f)
      continue;

    bool visible = true;
    glm::vec3 ray_dir = glm::normalize(to_player);
    for (const auto& aabb : colliders) {
      float t = 0.0f;
      glm::vec3 norm(0.0f);
      if (aabb.intersects_ray(eye_pos, ray_dir, t, norm)) {
        if (t > 0.4f && t < (dist_to_player - 0.4f)) {
          visible = false;
          break;
        }
      }
    }

    if (!visible)
      continue;

    glm::vec3 head_world = enemy->get_position() + glm::vec3(0.0f, 2.45f, 0.0f);
    glm::vec4 clip = VP * glm::vec4(head_world, 1.0f);
    if (clip.w > 0.1f) {
      glm::vec3 ndc = glm::vec3(clip) / clip.w;
      if (ndc.z >= -1.0f && ndc.z <= 1.0f && ndc.x >= -1.1f && ndc.x <= 1.1f &&
          ndc.y >= -1.1f && ndc.y <= 1.1f) {
        float sx = (ndc.x * 0.5f + 0.5f) * io.DisplaySize.x;
        float sy = (1.0f - (ndc.y * 0.5f + 0.5f)) * io.DisplaySize.y;

        float bar_w = glm::clamp(90.0f / (clip.w * 0.06f), 35.0f, 90.0f);
        float bar_h = 5.0f;
        float hp_pct = enemy->get_health() / enemy->get_max_health();

        draw->AddRectFilled(ImVec2(sx - bar_w * 0.5f, sy - bar_h * 0.5f),
                            ImVec2(sx + bar_w * 0.5f, sy + bar_h * 0.5f),
                            IM_COL32(10, 10, 10, 200),
                            2.5f);
        ImU32 fill_col = (enemy->get_damage_flash() > 0.0f)
                           ? IM_COL32(255, 255, 255, 255)
                           : IM_COL32(200, 40, 40, 230);
        draw->AddRectFilled(
          ImVec2(sx - bar_w * 0.5f, sy - bar_h * 0.5f),
          ImVec2(sx - bar_w * 0.5f + bar_w * hp_pct, sy + bar_h * 0.5f),
          fill_col,
          2.5f);
      }
    }
  }

  float cross_gap = 5.0f + m_gun_recoil * 50.0f;
  float cross_len = 6.0f;
  ImU32 cross_col = IM_COL32(240, 240, 240, 200);
  draw->AddLine(ImVec2(cx - cross_gap - cross_len, cy),
                ImVec2(cx - cross_gap, cy),
                cross_col,
                1.2f);
  draw->AddLine(ImVec2(cx + cross_gap, cy),
                ImVec2(cx + cross_gap + cross_len, cy),
                cross_col,
                1.2f);
  draw->AddLine(ImVec2(cx, cy - cross_gap - cross_len),
                ImVec2(cx, cy - cross_gap),
                cross_col,
                1.2f);
  draw->AddLine(ImVec2(cx, cy + cross_gap),
                ImVec2(cx, cy + cross_gap + cross_len),
                cross_col,
                1.2f);

  if (m_hitmarker_timer > 0.0f) {
    float h_size = 7.0f;
    ImU32 hit_col = IM_COL32(
      250, 250, 250, static_cast<int>((m_hitmarker_timer / 0.22f) * 240));
    draw->AddLine(ImVec2(cx - h_size, cy - h_size),
                  ImVec2(cx - 2.0f, cy - 2.0f),
                  hit_col,
                  1.5f);
    draw->AddLine(ImVec2(cx + 2.0f, cy + 2.0f),
                  ImVec2(cx + h_size, cy + h_size),
                  hit_col,
                  1.5f);
    draw->AddLine(ImVec2(cx + h_size, cy - h_size),
                  ImVec2(cx + 2.0f, cy - 2.0f),
                  hit_col,
                  1.5f);
    draw->AddLine(ImVec2(cx - 2.0f, cy + 2.0f),
                  ImVec2(cx - h_size, cy + h_size),
                  hit_col,
                  1.5f);
  }

  if (m_damage_flash_timer > 0.0f) {
    int alpha = static_cast<int>((m_damage_flash_timer / 0.28f) * 90);
    draw->AddRectFilled(
      ImVec2(0, 0), io.DisplaySize, IM_COL32(180, 20, 20, alpha));
  }

  float hp_ratio = m_player_health / m_player_max_health;
  float bar_w = 200.0f;
  draw->AddRectFilled(ImVec2(30, io.DisplaySize.y - 45),
                      ImVec2(30 + bar_w, io.DisplaySize.y - 31),
                      IM_COL32(15, 15, 15, 210),
                      3.0f);
  draw->AddRectFilled(ImVec2(30, io.DisplaySize.y - 45),
                      ImVec2(30 + bar_w * hp_ratio, io.DisplaySize.y - 31),
                      IM_COL32(40, 160, 70, 230),
                      3.0f);

  float map_cx = io.DisplaySize.x - 90.0f;
  float map_cy = io.DisplaySize.y - 90.0f;
  float map_r = 70.0f;
  draw->AddCircleFilled(
    ImVec2(map_cx, map_cy), map_r, IM_COL32(15, 15, 20, 220), 32);
  draw->AddCircle(
    ImVec2(map_cx, map_cy), map_r, IM_COL32(100, 100, 120, 200), 32, 2.0f);

  draw->AddCircleFilled(
    ImVec2(map_cx, map_cy), 4.0f, IM_COL32(60, 220, 90, 255));

  glm::vec2 scroll_delta(m_scroll_pos.x - player_pos.x,
                         m_scroll_pos.z - player_pos.z);
  if (glm::length(scroll_delta) < 160.0f) {
    float mx = map_cx + scroll_delta.x * (map_r / 160.0f);
    float my = map_cy + scroll_delta.y * (map_r / 160.0f);
    draw->AddCircleFilled(ImVec2(mx, my), 5.0f, IM_COL32(240, 210, 60, 255));
  }

  for (const auto& enemy : m_enemies) {
    if (enemy->is_dead())
      continue;
    glm::vec3 e_pos = enemy->get_position();
    glm::vec2 e_delta(e_pos.x - player_pos.x, e_pos.z - player_pos.z);
    if (glm::length(e_delta) < 160.0f) {
      float mx = map_cx + e_delta.x * (map_r / 160.0f);
      float my = map_cy + e_delta.y * (map_r / 160.0f);
      draw->AddCircleFilled(ImVec2(mx, my), 3.5f, IM_COL32(230, 50, 50, 240));
    }
  }

  if (m_game_state == GameState::PLAYING) {
    int dist = static_cast<int>(
      glm::distance(glm::vec2(player_pos.x, player_pos.z),
                    glm::vec2(m_scroll_pos.x, m_scroll_pos.z)));
    std::string text = "Scroll distance  " + std::to_string(dist) + "m";
    ImVec2 sz = ImGui::CalcTextSize(text.c_str());

    draw->AddText(nullptr,
                  18.0f,
                  ImVec2(cx - sz.x * 0.5f + 1.0f, 25.0f),
                  IM_COL32(0, 0, 0, 180),
                  text.c_str());
    draw->AddText(nullptr,
                  18.0f,
                  ImVec2(cx - sz.x * 0.5f, 24.0f),
                  IM_COL32(240, 220, 130, 240),
                  text.c_str());

  } else if (m_game_state == GameState::VICTORY) {
    const char* text = "You found the scroll";
    const char* sub = "Press R to play again";
    float text_y = io.DisplaySize.y * 0.38f;

    ImVec2 base_sz1 = ImGui::CalcTextSize(text);
    ImVec2 base_sz2 = ImGui::CalcTextSize(sub);
    float w1 = base_sz1.x * (32.0f / 14.0f);
    float w2 = base_sz2.x * (17.0f / 14.0f);

    draw->AddText(nullptr,
                  32.0f,
                  ImVec2(cx - w1 * 0.5f + 1.0f, text_y + 1.0f),
                  IM_COL32(0, 0, 0, 200),
                  text);
    draw->AddText(nullptr,
                  32.0f,
                  ImVec2(cx - w1 * 0.5f, text_y),
                  IM_COL32(110, 230, 140, 255),
                  text);

    draw->AddText(nullptr,
                  17.0f,
                  ImVec2(cx - w2 * 0.5f + 1.0f, text_y + 45.0f),
                  IM_COL32(0, 0, 0, 180),
                  sub);
    draw->AddText(nullptr,
                  17.0f,
                  ImVec2(cx - w2 * 0.5f, text_y + 44.0f),
                  IM_COL32(220, 220, 225, 230),
                  sub);

  } else if (m_game_state == GameState::DEFEATED) {
    const char* text = "You were taken down";
    const char* sub = "Press R to retry";
    float text_y = io.DisplaySize.y * 0.38f;

    ImVec2 base_sz1 = ImGui::CalcTextSize(text);
    ImVec2 base_sz2 = ImGui::CalcTextSize(sub);
    float w1 = base_sz1.x * (32.0f / 14.0f);
    float w2 = base_sz2.x * (17.0f / 14.0f);

    draw->AddText(nullptr,
                  32.0f,
                  ImVec2(cx - w1 * 0.5f + 1.0f, text_y + 1.0f),
                  IM_COL32(0, 0, 0, 200),
                  text);
    draw->AddText(nullptr,
                  32.0f,
                  ImVec2(cx - w1 * 0.5f, text_y),
                  IM_COL32(235, 80, 80, 255),
                  text);

    draw->AddText(nullptr,
                  17.0f,
                  ImVec2(cx - w2 * 0.5f + 1.0f, text_y + 45.0f),
                  IM_COL32(0, 0, 0, 180),
                  sub);
    draw->AddText(nullptr,
                  17.0f,
                  ImVec2(cx - w2 * 0.5f, text_y + 44.0f),
                  IM_COL32(220, 220, 225, 230),
                  sub);
  }
}
