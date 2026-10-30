#include "Enemy.h"
#include "Material.h"
#include "Mesh.h"
#include "Scene.h"
#include "ShaderProgram.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

Enemy::Enemy(Scene& scene,
             const glm::vec3& spawn_pos,
             const std::vector<glm::vec3>& waypoints)
  : m_spawn_point(spawn_pos)
  , m_waypoints(waypoints)
{
  m_root = std::make_shared<SceneObject>();
  m_root->get_local_transform().set_position(spawn_pos);
  m_root->get_physics_component().disable();
  m_root->set_collidable(false);

  const glm::vec3 c_armor = { 0.68f, 0.69f, 0.70f };
  const glm::vec3 c_visor = { 0.95f, 0.20f, 0.20f };
  const glm::vec3 c_gun = { 0.16f, 0.16f, 0.18f };

  auto create_part = [&](const glm::vec3& offset,
                         const glm::vec3& scale,
                         const glm::vec3& col) {
    auto part = std::make_shared<SceneObject>();
    part->get_local_transform().set_position(offset).set_scale(scale);
    part->get_physics_component().disable();
    part->set_collidable(false);

    std::string name = "enm_" + std::to_string(m_id) + "_" +
                       std::to_string(offset.x) + "_" +
                       std::to_string(offset.y);

    std::weak_ptr<EnemyVisualState> visual_state = m_visual_state;
    part->set_render_packet(
      scene.get_app_cache().mesh_repo.get("sharp_box_mesh").value(),
      scene.get_app_cache().material_repo.load_material(MaterialDescriptor{
        .material_name = name + "_mat",
        .shader_program_desc = { .program_name = "cube_program",
                                 .shaders = { "cube.vert", "cube.frag" } },
        .texture_desc_list = {},
        .uniforms = { { "u_color", col },
                      { "u_use_texture", 0 },
                      { "u_is_gun", 0 } } }),
      capture_weak(
        part,
        [&scene, col, visual_state](SceneObjectStrongPtr self,
                                    RenderPacketStrongPtr packet) {
          scene.set_view_matrix(packet);
          scene.set_projection_matrix(packet);
          scene.set_model_matrix(packet, self->get_world_transformation_mat());

          glm::vec3 active_color = col;
          const auto state = visual_state.lock();
          if (state) {
            if (state->state == EnemyState::DEAD) {
              active_color = { 0.4823529411764706f,
                               0.06666666666666667f,
                               0.07450980392156863f };
            } else if (state->damage_flash > 0.0f) {
              active_color = glm::mix(col, glm::vec3(1.0f, 0.1f, 0.1f), 0.75f);
            }
          }
          auto shader_program = packet->material->get_shader_program();
          shader_program->set_uniform("u_color", active_color);
          shader_program->set_uniform("u_preserve_color", 1);
          packet->mesh->draw();
          shader_program->set_uniform("u_preserve_color", 0);
        }));

    m_root->add_child(part);
    m_body_parts.push_back(part);
    return part;
  };

  create_part({ -0.25f, 0.5f, 0.0f }, { 0.32f, 1.0f, 0.32f }, c_armor);
  create_part({ 0.25f, 0.5f, 0.0f }, { 0.32f, 1.0f, 0.32f }, c_armor);
  create_part({ 0.0f, 1.35f, 0.0f }, { 0.85f, 0.80f, 0.50f }, c_armor);
  create_part({ 0.0f, 2.05f, 0.0f }, { 0.52f, 0.52f, 0.52f }, c_armor);
  create_part({ 0.0f, 2.05f, -0.28f }, { 0.42f, 0.14f, 0.08f }, c_visor);

  create_part({ 0.30f, 1.25f, -0.45f }, { 0.10f, 0.16f, 0.95f }, c_gun);
  create_part({ 0.30f, 1.12f, -0.32f }, { 0.08f, 0.26f, 0.12f }, c_gun);
  create_part({ 0.30f, 1.38f, -0.30f }, { 0.08f, 0.11f, 0.30f }, c_gun);

  if (!m_waypoints.empty()) {
    m_target_pos = m_waypoints[0];
  }
}

glm::vec3
Enemy::get_position() const
{
  return m_root->get_local_transform().get_position();
}

BoundingBox
Enemy::get_hitbox() const
{
  glm::vec3 pos = get_position();
  return BoundingBox({ pos.x - 0.5f, pos.y, pos.z - 0.5f },
                     { pos.x + 0.5f, pos.y + 2.3f, pos.z + 0.5f });
}

bool
Enemy::has_line_of_sight(const glm::vec3& player_pos, const std::vector<BoundingBox>& colliders) const
{
  glm::vec3 eye_pos = get_position() + glm::vec3(0.0f, 1.8f, 0.0f);
  glm::vec3 to_player = player_pos - eye_pos;
  float dist_to_player = glm::length(to_player);

  if (dist_to_player > 40.0f)
    return false;

  glm::vec3 ray_dir = glm::normalize(to_player);

  for (const auto& aabb : colliders) {
    float t = 0.0f;
    glm::vec3 normal(0.0f);
    if (aabb.intersects_ray(eye_pos, ray_dir, t, normal)) {
      if (t > 0.5f && t < dist_to_player - 0.5f) {
        return false; // Real wall in between
      }
    }
  }

  return true;
}

bool
Enemy::take_damage(float damage, const glm::vec3& impulse)
{
  if (m_state == EnemyState::DEAD)
    return false;

  m_health -= damage;
  m_state = EnemyState::HUNT;
  m_root->get_local_transform().translate(impulse);

  if (m_health <= 0.0f) {
    m_health = 0.0f;
    m_state = EnemyState::DEAD;
    m_root->get_local_transform().rotate_deg(AXIS_X, 85.0f);
    m_root->get_local_transform().translate({ 0.0f, -0.8f, 0.0f });
    return true;
  }

  return false;
}

void
Enemy::update(Scene& scene, float dt)
{
  if (m_state == EnemyState::DEAD)
    return;

  auto player = scene.get_player();
  if (!player)
    return;

  glm::vec3 player_pos = player->get_local_transform().get_position();
  glm::vec3 pos = get_position();

  std::vector<BoundingBox> colliders;
  scene.collect_colliders_recursive(scene.get_scene_ptr(), colliders);

  float dist_to_player = glm::distance(glm::vec2(pos.x, pos.z),
                                       glm::vec2(player_pos.x, player_pos.z));
  bool can_see = has_line_of_sight(player_pos, colliders);

  // Hearing radius (14 meters) or direct line of sight
  if (can_see || dist_to_player < 14.0f) {
    m_state = EnemyState::HUNT;
    m_target_pos = player_pos;
    m_alert_timer = 6.0f;
  } else if (m_state == EnemyState::HUNT) {
    m_alert_timer -= dt;
    if (m_alert_timer <= 0.0f) {
      m_state = EnemyState::PATROL;
    }
  }

  if (m_state == EnemyState::PATROL && !m_waypoints.empty()) {
    if (glm::distance(glm::vec2(pos.x, pos.z),
                      glm::vec2(m_target_pos.x, m_target_pos.z)) < 1.5f) {
      m_current_waypoint = (m_current_waypoint + 1) % m_waypoints.size();
      m_target_pos = m_waypoints[m_current_waypoint];
    }
  }

  glm::vec3 to_target = m_target_pos - pos;
  to_target.y = 0.0f;
  float dist = glm::length(to_target);

  if (dist > 0.4f) {
    glm::vec3 dir = glm::normalize(to_target);
    float current_speed =
      (m_state == EnemyState::HUNT) ? m_speed * 1.6f : m_speed;
    glm::vec3 delta = dir * current_speed * dt;

    auto can_move = [&](float tx, float tz) -> bool {
      for (const auto& aabb : colliders) {
        const glm::vec3& b_min = aabb.get_min();
        const glm::vec3& b_max = aabb.get_max();

        if (tx + 0.35f > b_min.x && tx - 0.35f < b_max.x &&
            tz + 0.35f > b_min.z && tz - 0.35f < b_max.z) {
          if (b_max.y > pos.y + 0.5f && b_min.y < pos.y + 2.0f)
            return false;
        }
      }
      return true;
    };

    glm::vec3 new_pos = pos;
    // Slide along X if open
    if (can_move(pos.x + delta.x, pos.z))
      new_pos.x += delta.x;
    // Slide along Z if open
    if (can_move(new_pos.x, pos.z + delta.z))
      new_pos.z += delta.z;

    m_root->get_local_transform().set_position(new_pos);

    float yaw = atan2(-dir.x, -dir.z);
    m_root->get_local_transform().set_rotation(
      glm::quat(glm::vec3(0.0f, yaw, 0.0f)));
  }
}
