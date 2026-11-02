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
RivalGrounds::handle_object_grab()
{
  auto& window = m_app_cache.get_window();
  bool right_pressed = window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);

  if (right_pressed && !m_right_mouse_was_pressed) {
    if (m_held_object) {
      glm::vec3 origin = m_player->get_local_transform().get_position();
      glm::vec3 dir = m_app_cache.get_camera().get_forward();

      const float CRATE_SIZE = 1.80f;
      const float CRATE_HALF = 0.90f;

      auto objects = get_all_physics_objects();
      float closest_t = 4.2f;
      SceneObjectStrongPtr hit_box = nullptr;
      glm::vec3 hit_norm(0.0f, 1.0f, 0.0f);

      for (auto& obj : objects) {
        if (obj == m_held_object)
          continue;
        float t = 0.0f;
        glm::vec3 norm(0.0f);
        if (obj->get_world_AABB().intersects_ray(origin, dir, t, norm)) {
          if (t > 0.2f && t < closest_t) {
            closest_t = t;
            hit_box = obj;
            hit_norm = norm;
          }
        }
      }

      glm::vec3 place_pos(0.0f);

      if (hit_box) {

        glm::vec3 base_pos = hit_box->get_local_transform().get_position();
        if (hit_norm.y > 0.4f) {

          place_pos = base_pos + glm::vec3(0.0f, CRATE_SIZE, 0.0f);
        } else {

          glm::vec3 flat_norm =
            glm::normalize(glm::vec3(hit_norm.x, 0.0f, hit_norm.z));
          place_pos = base_pos + flat_norm * CRATE_SIZE;
          place_pos.y = base_pos.y;
        }
      } else {

        place_pos = origin + dir * 2.2f;
        place_pos.y = CRATE_HALF;
      }

      m_held_object->get_local_transform().set_position(place_pos).set_rotation(
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

      m_held_object->set_collidable(true);
      auto& phys = m_held_object->get_physics_component();
      phys.enable();
      phys.set_gravity(true);
      phys.set_linear_momentum({ 0.0f, 0.0f, 0.0f });
      phys.set_angular_momentum({ 0.0f, 0.0f, 0.0f });

      m_held_object = nullptr;
    } else {

      glm::vec3 origin = m_player->get_local_transform().get_position();
      glm::vec3 dir = m_app_cache.get_camera().get_forward();

      auto objects = get_all_physics_objects();
      SceneObjectStrongPtr closest_obj = nullptr;
      float closest_dist = 4.0f;

      for (auto& obj : objects) {
        float t = 0.0f;
        glm::vec3 norm(0.0f);
        if (obj->get_world_AABB().intersects_ray(origin, dir, t, norm)) {
          if (t > 0.2f && t < closest_dist) {
            closest_dist = t;
            closest_obj = obj;
          }
        }
      }

      if (closest_obj) {
        m_held_object = closest_obj;

        m_held_object->set_collidable(false);
        m_held_object->get_physics_component().disable();
      }
    }
  }
  m_right_mouse_was_pressed = right_pressed;

  if (m_held_object) {
    glm::vec3 hold_pos = m_player->get_local_transform().get_position() +
                         m_app_cache.get_camera().get_forward() * 2.2f +
                         glm::vec3(0.0f, -0.1f, 0.0f);
    m_held_object->get_local_transform().set_position(hold_pos).set_rotation(
      glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    m_held_object->visible = true;
  }
}

void
RivalGrounds::handle_shooting()
{
  auto& window = m_app_cache.get_window();
  bool is_pressed = window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);

  if (is_pressed && !m_mouse_was_pressed) {
    glm::vec3 origin = m_player->get_local_transform().get_position();
    glm::vec3 dir = m_app_cache.get_camera().get_forward();

    m_gun_recoil = 0.11f;

    std::vector<BoundingBox> colliders;
    collect_game_colliders(colliders, false);

    float env_t = 200.0f;
    float mesh_t = 0.0f;
    if (static_mesh_ray_hit(origin, dir, env_t, mesh_t))
      env_t = mesh_t;
    for (const auto& aabb : colliders) {
      float t = 0.0f;
      glm::vec3 norm(0.0f);
      if (aabb.intersects_ray(origin, dir, t, norm)) {
        if (t > 0.2f && t < env_t) {
          env_t = t;
        }
      }
    }

    bool hit_enemy = false;
    for (auto& enemy : m_enemies) {
      if (!enemy->is_dead()) {
        float t = 0.0f;
        glm::vec3 norm(0.0f);
        if (enemy->get_hitbox().intersects_ray(origin, dir, t, norm)) {
          if (t < env_t) {
            env_t = t;
            m_hitmarker_timer = 0.22f;
            enemy->take_damage(*this, 65.0f, dir * 0.4f, colliders);
            m_tracers.push_back({ origin + dir * 0.4f,
                                  origin + dir * t,
                                  0.08f,
                                  glm::vec3(0.9f, 0.8f, 0.3f) });
            hit_enemy = true;
          }
        }
      }
    }

    if (!hit_enemy) {
      m_tracers.push_back({ origin + dir * 0.4f,
                            origin + dir * env_t,
                            0.08f,
                            glm::vec3(0.9f, 0.8f, 0.3f) });
    }

    auto objects = get_all_physics_objects();
    for (auto& obj : objects) {
      float t = 0.0f;
      glm::vec3 normal(0.0f);
      if (obj->get_world_AABB().intersects_ray(origin, dir, t, normal)) {
        if (t <= env_t) {
          glm::vec3 hit_pt = origin + dir * t;
          obj->get_physics_component().apply_impulse_at_point(
            dir * 8.0f + glm::vec3(0, 1.2f, 0), hit_pt);
          obj->get_physics_component().set_angular_momentum(
            glm::vec3(0.4f, 0.6f, 0.2f));
        }
      }
    }
  }
  m_mouse_was_pressed = is_pressed;
}
