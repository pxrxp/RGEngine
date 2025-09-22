#include "Camera.h"
#include "App.h"
#include "Renderer.h"
#include "Scene.h"
#include "components.h"
#include "definitions.h"
#include "opengl.h"
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

glm::vec3 Camera::get_camera_move_dir(App &app) const {
  glm::vec3 direction(0.f);

  bool go_forward = app.get_window().is_key_pressed(GLFW_KEY_W) ||
                    app.get_window().is_key_pressed(GLFW_KEY_UP);
  bool go_backward = app.get_window().is_key_pressed(GLFW_KEY_S) ||
                     app.get_window().is_key_pressed(GLFW_KEY_DOWN);
  bool go_left = app.get_window().is_key_pressed(GLFW_KEY_A) ||
                 app.get_window().is_key_pressed(GLFW_KEY_LEFT);
  bool go_right = app.get_window().is_key_pressed(GLFW_KEY_D) ||
                  app.get_window().is_key_pressed(GLFW_KEY_RIGHT);
  bool go_up = app.get_window().is_key_pressed(GLFW_KEY_Q);
  bool go_down = app.get_window().is_key_pressed(GLFW_KEY_E);
  bool go_fast = app.get_window().is_key_pressed(GLFW_KEY_LEFT_CONTROL) ||
                 app.get_window().is_key_pressed(GLFW_KEY_RIGHT_CONTROL);
  bool go_slow = app.get_window().is_key_pressed(GLFW_KEY_LEFT_SHIFT) ||
                 app.get_window().is_key_pressed(GLFW_KEY_RIGHT_SHIFT);

  if (go_forward)  direction.z += 1.0f;
  if (go_backward) direction.z -= 1.0f;
  if (go_left)     direction.x -= 1.0f;
  if (go_right)    direction.x += 1.0f;
  if (go_up)       direction.y += 1.0f;
  if (go_down)     direction.y -= 1.0f;

  if (glm::length(direction) > 0.0f) {
    direction = glm::normalize(direction);
  }

  if (go_fast) direction *= 2.0f;
  if (go_slow) direction /= 2.0f;

  return direction;
}

void Camera::setup(SceneObjectPtr player, float aspect_ratio,
                   float thrust_strength, float sensitivity, float fovy_deg, float z_near,
                   float z_far) {
  m_target_player = player;
  m_thrust_strength = thrust_strength;
  m_sensitivity = glm::max(0.f, sensitivity);
  m_fovy_rad = glm::radians(fovy_deg);
  m_proj = glm::perspective(m_fovy_rad, aspect_ratio, z_near, z_far);

  float top = z_far * tan(m_fovy_rad / 2);
  float right = (z_far / z_near) * top * aspect_ratio;
  
  m_bounding_box = {
    .min = {-right, -top, z_near},
    .max = {right, top, z_far}
  };
}

Boundary Camera::get_bounding_box() const {
  return m_bounding_box;
}

void Camera::update(App &app) {
  if (!m_target_player) return;

  auto delta_mouse = app.get_window().get_delta_mouse();
  if (delta_mouse.x != 0 || delta_mouse.y != 0) {
    float px_to_rad = m_fovy_rad / app.get_window().get_height();
    m_yaw_rad += -delta_mouse.x * px_to_rad * m_sensitivity;
    m_pitch_rad += delta_mouse.y * px_to_rad * m_sensitivity;
    const float pitch_limit = glm::pi<float>() / 2.0f - 0.01f;
    m_pitch_rad = glm::clamp(m_pitch_rad, -pitch_limit, pitch_limit);
    m_dirty = true;
  }
  
  glm::quat player_orientation = glm::quat({m_pitch_rad, m_yaw_rad, 0.0f});
  m_target_player->m_local_transform.set_rotation(player_orientation);

  auto move_dir = get_camera_move_dir(app);
  if (glm::length(move_dir) > 0.0f) {
    glm::vec3 forward = player_orientation * glm::vec3(AXIS_NEG_Z);
    glm::vec3 right = player_orientation * glm::vec3(AXIS_X);
    glm::vec3 up = glm::vec3(AXIS_Y);

    // glm::vec3 world_space_move_dir = right * move_dir.x + up * move_dir.y + forward * move_dir.z;
    // glm::vec3 offset = world_space_move_dir * m_speed * app.get_delta_time();
    // m_target_player->m_local_transform.translate(offset);

    glm::vec3 move_force = (right * move_dir.x + up * move_dir.y + forward * move_dir.z) * m_thrust_strength;
    m_target_player->m_physics.apply_force(move_force);
    m_dirty = true;
  }
}

void Camera::late_update() {
  if (!m_target_player) return;

  glm::vec3 player_pos = m_target_player->m_local_transform.get_position();
  glm::quat player_orientation = m_target_player->m_local_transform.get_rotation();
  glm::vec3 forward = player_orientation * glm::vec3(AXIS_NEG_Z);

  m_view = glm::lookAt(player_pos, player_pos + forward, glm::vec3(AXIS_Y));
  
  m_dirty = false;
}

glm::mat4 Camera::get_view_matrix() {
  if (m_dirty) {
    late_update();
  }
  return m_view;
}

glm::mat4 Camera::get_projection_matrix() const { return m_proj; }

float Camera::get_fovy_rad() const { return m_fovy_rad; }

float Camera::get_sensitivity() const { return m_sensitivity; }

void Camera::set_sensitivity(float sensitivity) {
  m_sensitivity = glm::max(0.f, sensitivity);
}
