#pragma once
#include "components.h"
#include <glm/glm.hpp>

class App;

class Camera {
  float m_sensitivity = 0.f;
  float m_yaw_rad = 0.f;
  float m_pitch_rad = 0.f;
  float m_fovy_rad = 0.f;

  glm::mat4 m_view = glm::mat4(1.f);
  glm::mat4 m_proj = glm::mat4(1.f);

public:
  TransformComponent transform;
  PhysicsComponent physics;
  
  void setup(glm::vec3 position, glm::vec3 target, float aspect_ratio,
             float sensitivity, glm::vec3 speed, glm::vec3 up = {0.f, 1.f, 0.f},
             float fovy_deg = 60.f, float z_near = 0.1f, float z_far = 100.0f);

  glm::mat4 get_view_matrix() const;
  glm::mat4 get_projection_matrix() const;

  float get_fovy_rad() const;
  float get_sensitivity() const;
  void set_sensitivity(float sensitivity);

  friend class App;
};
