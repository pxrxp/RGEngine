#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

void Camera::setup(glm::vec3 position, glm::vec3 target, float aspect_ratio,
                   float sensitivity, glm::vec3 speed, glm::vec3 up,
                   float fovy_deg, float z_near, float z_far) {
  m_sensitivity = glm::max(0.f, sensitivity);
  m_fovy_rad = glm::radians(fovy_deg);
  m_pos = position;
  m_speed = speed;

  glm::vec3 forward = glm::normalize(target - position);
  m_pitch_rad = asin(forward.y);
  m_yaw_rad = atan2(forward.z, forward.x);

  m_view = glm::lookAt(m_pos, target, up);
  m_proj = glm::perspective(m_fovy_rad, aspect_ratio, z_near, z_far);
}

glm::mat4 Camera::get_view_matrix() const { return m_view; }

glm::mat4 Camera::get_projection_matrix() const { return m_proj; }

float Camera::get_fovy_rad() const { return m_fovy_rad; }
glm::vec3 Camera::get_speed() const { return m_speed; }
float Camera::get_sensitivity() const { return m_sensitivity; }
void Camera::set_sensitivity(float sensitivity) {
  m_sensitivity = glm::max(0.f, sensitivity);
}
