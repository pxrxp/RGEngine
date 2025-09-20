#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

void Camera::setup(glm::vec3 position, glm::vec3 target, float aspect_ratio,
                   float sensitivity, glm::vec3 speed, glm::vec3 up,
                   float fovy_deg, float z_near, float z_far) {
  this->sensitivity = glm::max(0.f, sensitivity);
  this->fovy_rad = glm::radians(fovy_deg);
  this->pos = position;
  this->speed = speed;

  glm::vec3 forward = glm::normalize(target - position);
  this->pitch_rad = asin(forward.y);
  this->yaw_rad = atan2(forward.z, forward.x);

  this->view = glm::lookAt(this->pos, target, up);
  this->proj = glm::perspective(this->fovy_rad, aspect_ratio, z_near, z_far);
}

glm::mat4 Camera::get_view_matrix() const { return this->view; }

glm::mat4 Camera::get_projection_matrix() const { return this->proj; }

float Camera::get_fovy_rad() const { return this->fovy_rad; }
glm::vec3 Camera::get_speed() const { return this->speed; }
float Camera::get_sensitivity() const { return this->sensitivity; }
void Camera::set_sensitivity(float sensitivity) {
  this->sensitivity = glm::max(0.f, sensitivity);
}
