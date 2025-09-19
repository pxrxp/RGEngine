#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

void Camera::setup(glm::vec3 position, glm::vec3 target, float aspect_ratio,
                   glm::vec3 up, float fovy, float z_near, float z_far) {

  this->view = glm::lookAt(position, target, up);
  this->proj =
      glm::perspective(glm::radians(fovy), aspect_ratio, z_near, z_far);
}

glm::mat4 Camera::get_view_matrix() const { return view; }

glm::mat4 Camera::get_projection_matrix() const { return proj; }
