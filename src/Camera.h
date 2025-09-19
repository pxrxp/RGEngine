#pragma once
#include <glm/glm.hpp>

class App;

class Camera {
  glm::vec3 pos;
  float yaw = 0.f;
  float pitch = 0.f;

  glm::mat4 view;

public:
  glm::mat4& get_view_matrix() {
    return view;
  }

  friend class App;
};
