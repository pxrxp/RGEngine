#include <rivalgrounds/Enemy.h>
#include <engine/Material.h>
#include <engine/Mesh.h>
#include <engine/Scene.h>
#include <engine/ShaderProgram.h>
#include <glm/gtc/matrix_transform.hpp>

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

  scene.get_scene_ptr()->add_child(m_root);
  m_target_pos = m_waypoints.empty() ? spawn_pos : m_waypoints[0];
  m_visual_state->state = m_state;
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
  return BoundingBox({ pos.x - 0.55f, pos.y, pos.z - 0.55f },
                     { pos.x + 0.55f, pos.y + 2.3f, pos.z + 0.55f });
}

bool
Enemy::has_line_of_sight(const Scene& scene,
                         const glm::vec3& player_pos,
                         const std::vector<BoundingBox>& colliders) const
{
  glm::vec3 eye_pos = get_position() + glm::vec3(0.0f, 1.9f, 0.0f);
  glm::vec3 to_player = player_pos - eye_pos;
  float dist = glm::length(to_player);

  if (dist > MAX_VISION_RANGE || dist < 0.1f)
    return false;

  glm::vec3 ray_dir = to_player / dist;

  if (std::abs(ray_dir.y) > MAX_VERTICAL_AIM)
    return false;

  glm::quat rot = m_root->get_local_transform().get_rotation();
  glm::vec3 forward = rot * glm::vec3(0.0f, 0.0f, -1.0f);
  if (m_state == EnemyState::PATROL && glm::dot(forward, ray_dir) < FOV_COSINE)
    return false;

  for (const auto& aabb : colliders) {
    float t = 0.0f;
    glm::vec3 normal(0.0f);
    if (aabb.intersects_ray(eye_pos, ray_dir, t, normal)) {
      if (t > 0.4f && t < (dist - 0.4f)) {
        return false;
      }
    }
  }

  float hit_t = 0.0f;
  if (scene.raycast_static_geometry(eye_pos, ray_dir, dist - 0.4f, hit_t) &&
      hit_t > 0.4f)
    return false;

  return true;
}

bool
Enemy::take_damage(Scene& scene,
                   float damage,
                   const glm::vec3& impulse,
                   const std::vector<BoundingBox>& colliders)
{
  if (m_state == EnemyState::DEAD)
    return false;

  m_health -= damage;
  m_state = EnemyState::COMBAT;
  m_alert_timer = 6.0f;
  m_regen_cooldown = 4.0f;
  m_damage_flash = 0.22f;

  glm::vec3 current_pos = get_position();
  glm::vec3 test_pos = current_pos + impulse * ENEMY_KNOCKBACK_SCALE;

  bool blocked = false;
  for (const auto& aabb : colliders) {
    const glm::vec3& b_min = aabb.get_min();
    const glm::vec3& b_max = aabb.get_max();
    if (test_pos.x + 0.4f > b_min.x && test_pos.x - 0.4f < b_max.x &&
        test_pos.z + 0.4f > b_min.z && test_pos.z - 0.4f < b_max.z) {
      if (b_max.y > current_pos.y + 0.2f && b_min.y < current_pos.y + 2.0f) {
        blocked = true;
        break;
      }
    }
  }

  if (!blocked) {
    blocked = scene.blocks_vertical_capsule_for_actor(test_pos.x,
                                                      test_pos.z,
                                                      current_pos.x,
                                                      current_pos.z,
                                                      current_pos.y,
                                                      current_pos.y +
                                                        ENEMY_COLLISION_HEIGHT,
                                                      ENEMY_COLLISION_RADIUS);
  }

  if (!blocked)
    m_root->get_local_transform().set_position(test_pos);

  if (m_health <= 0.0f) {
    m_health = 0.0f;
    m_state = EnemyState::DEAD;
    m_visual_state->state = m_state;
    m_visual_state->damage_flash = 0.0f;
    m_root->get_local_transform().set_position(current_pos +
                                               glm::vec3{ 0, 0.5, 0 });
    m_root->get_local_transform().rotate_deg(AXIS_X, 90.0f);
    return true;
  }

  return false;
}

void
Enemy::update(Scene& scene,
              const glm::vec3& player_pos,
              const glm::vec3& player_cam_fwd,
              const std::vector<BoundingBox>& colliders,
              float dt,
              float& out_damage,
              bool& out_fired_shot,
              glm::vec3& out_gun_pos)
{
  (void)player_cam_fwd;
  out_damage = 0.0f;
  out_fired_shot = false;

  if (m_state == EnemyState::DEAD)
    return;

  if (m_damage_flash > 0.0f)
    m_damage_flash = std::max(0.0f, m_damage_flash - dt);
  m_visual_state->state = m_state;
  m_visual_state->damage_flash = m_damage_flash;

  if (m_shoot_cooldown > 0.0f)
    m_shoot_cooldown -= dt;

  if (m_regen_cooldown > 0.0f) {
    m_regen_cooldown -= dt;
  } else if (m_health < m_max_health) {
    m_health = std::min(m_max_health, m_health + 15.0f * dt);
  }

  glm::vec3 pos = get_position();
  out_gun_pos = pos + glm::vec3(0.30f, 1.25f, -0.45f);

  float dist_to_player = glm::distance(glm::vec2(pos.x, pos.z),
                                       glm::vec2(player_pos.x, player_pos.z));

  const bool player_heard = dist_to_player <= HEARING_DISTANCE;
  const bool can_see = has_line_of_sight(scene, player_pos, colliders);

  if (can_see || player_heard) {
    m_state = EnemyState::COMBAT;
    m_alert_timer = 7.0f;
  }

  if (m_state == EnemyState::COMBAT) {
    m_alert_timer -= dt;
    if (m_alert_timer <= 0.0f && !can_see && !player_heard) {
      m_state = EnemyState::PATROL;
      m_target_pos = m_waypoints.empty() ? m_spawn_point : m_waypoints[0];
    } else {

      glm::vec3 to_p = player_pos - pos;
      to_p.y = 0.0f;
      float dist_h = glm::length(to_p);

      if (dist_h > 0.05f) {
        glm::vec3 look_dir = to_p / dist_h;
        float target_yaw = std::atan2(-look_dir.x, -look_dir.z);
        m_root->get_local_transform().set_rotation(
          glm::quat(glm::vec3(0.0f, target_yaw, 0.0f)));
      }

      if (dist_h > 18.0f) {
        m_target_pos = player_pos;
      } else if (dist_h < 7.0f) {
        m_target_pos = pos - glm::normalize(to_p) * 6.0f;
      } else {
        glm::vec3 right = glm::normalize(
          glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(to_p)));
        m_target_pos = pos + right * 3.5f;
      }

      glm::vec3 aim_dir = glm::normalize(player_pos - out_gun_pos);
      if (std::abs(aim_dir.y) <= MAX_VERTICAL_AIM && m_shoot_cooldown <= 0.0f &&
          dist_to_player <= 36.0f) {
        out_fired_shot = true;
        out_damage = 10.0f;
        m_shoot_cooldown = 1.40f;
      }
    }
  }

  if (m_state == EnemyState::PATROL && !m_waypoints.empty()) {
    float to_wp_dist = glm::distance(glm::vec2(pos.x, pos.z),
                                     glm::vec2(m_target_pos.x, m_target_pos.z));
    if (to_wp_dist < 2.5f) {
      m_current_waypoint = (m_current_waypoint + 1) % m_waypoints.size();
      m_target_pos = m_waypoints[m_current_waypoint];
    }
  }

  glm::vec3 to_target = m_target_pos - pos;
  to_target.y = 0.0f;
  float dist_to_target = glm::length(to_target);

  if (dist_to_target > 1.2f) {
    glm::vec3 dir = to_target / dist_to_target;
    float current_speed = (m_state == EnemyState::COMBAT) ? 3.4f : 2.0f;
    glm::vec3 delta = dir * current_speed * dt;

    auto test_step =
      [&](float nx, float nz, float from_x, float from_z) -> bool {
      for (const auto& aabb : colliders) {
        const glm::vec3& b_min = aabb.get_min();
        const glm::vec3& b_max = aabb.get_max();
        if (nx + 0.5f > b_min.x && nx - 0.5f < b_max.x && nz + 0.5f > b_min.z &&
            nz - 0.5f < b_max.z) {
          if (b_max.y > pos.y + 0.3f &&
              b_min.y < pos.y + ENEMY_COLLISION_HEIGHT)
            return false;
        }
      }
      if (scene.blocks_vertical_capsule_for_actor(nx,
                                                  nz,
                                                  from_x,
                                                  from_z,
                                                  pos.y,
                                                  pos.y +
                                                    ENEMY_COLLISION_HEIGHT,
                                                  ENEMY_COLLISION_RADIUS))
        return false;
      return true;
    };

    glm::vec3 new_pos = pos;
    const bool moved_x = test_step(pos.x + delta.x, pos.z, pos.x, pos.z);
    if (moved_x)
      new_pos.x += delta.x;

    const bool moved_z =
      test_step(new_pos.x, pos.z + delta.z, new_pos.x, pos.z);
    if (moved_z)
      new_pos.z += delta.z;

    if (!moved_x && !moved_z && m_state == EnemyState::PATROL &&
        !m_waypoints.empty()) {
      m_current_waypoint = (m_current_waypoint + 1) % m_waypoints.size();
      m_target_pos = m_waypoints[m_current_waypoint];
    }

    float ground_y = new_pos.y;
    if (scene.find_actor_ground_surface(
          new_pos.x, new_pos.z, pos.y + 1.0f, pos.y - 2.0f, ground_y))
      new_pos.y = ground_y;
    m_root->get_local_transform().set_position(new_pos);

    if (m_state != EnemyState::COMBAT) {
      float yaw = std::atan2(-dir.x, -dir.z);
      m_root->get_local_transform().set_rotation(
        glm::quat(glm::vec3(0.0f, yaw, 0.0f)));
    }
  }
}
