#include "Camera.h"
#include "definitions.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

void Camera::setup(glm::vec3 position, glm::vec3 target, float aspect_ratio,
                   float sensitivity, glm::vec3 speed, glm::vec3 up,
                   float fovy_deg, float z_near, float z_far) {
  m_sensitivity = glm::max(0.f, sensitivity);
  m_fovy_rad = glm::radians(fovy_deg);
  transform.position = position;
  physics.speed = speed;

  glm::vec3 forward = glm::normalize(target - position);
  m_pitch_rad = asin(forward.y);
  m_yaw_rad = atan2(forward.z, forward.x);

  m_view = glm::lookAt(transform.position, target, up);
  m_proj = glm::perspective(m_fovy_rad, aspect_ratio, z_near, z_far);
}

void Camera::process_mouse_movement(float delta_x, float delta_y) {
  float yaw_angle = -delta_x * m_sensitivity;
  glm::quat yaw_delta = glm::angleAxis(yaw_angle, glm::vec3(AXIS_Y));
  glm::quat new_orientation = yaw_delta * transform.orientation;

  float pitch_angle = -delta_y * m_sensitivity;
  glm::vec3 right_vector = new_orientation * glm::vec3(AXIS_X);
  glm::quat pitch_delta = glm::angleAxis(pitch_angle, right_vector);

  glm::quat potential_orientation = pitch_delta * new_orientation;
  glm::vec3 potential_forward = potential_orientation * glm::vec3(0.0f, 0.0f, -1.0f);

  const float pitch_limit = glm::sin(glm::radians(89.0f));
  if (glm::abs(potential_forward.y) < pitch_limit) {
    transform.orientation = potential_orientation;
  } else {
    transform.orientation = new_orientation;
  }

  transform.orientation = glm::normalize(transform.orientation);
}

glm::mat4 Camera::get_view_matrix() const { return m_view; }

glm::mat4 Camera::get_projection_matrix() const { return m_proj; }

float Camera::get_fovy_rad() const { return m_fovy_rad; }

float Camera::get_sensitivity() const { return m_sensitivity; }
void Camera::set_sensitivity(float sensitivity) {
  m_sensitivity = glm::max(0.f, sensitivity);
}
