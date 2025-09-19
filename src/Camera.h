#pragma once
#include <glm/glm.hpp>

class App;

class Camera {
  glm::vec3 pos;
  float yaw = 0.f;
  float pitch = 0.f;
  float sensitivity = 0.f;

  glm::mat4 view = glm::mat4(1.f);
  glm::mat4 proj = glm::mat4(1.f);

public:
  void setup(glm::vec3 position, glm::vec3 target, float aspect_ratio,
             float sensitivity = 10.f, glm::vec3 up = {0.f, 1.f, 0.f},
             float fovy = 60.f, float z_near = 0.1f, float z_far = 100.0f);

  glm::mat4 get_view_matrix() const;
  glm::mat4 get_projection_matrix() const;

  friend class App;
};
