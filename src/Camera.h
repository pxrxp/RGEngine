#pragma once
#include "Renderer.h"
#include <glm/glm.hpp>

class App;

class Camera {

  SceneObjectPtr m_target_player;

  float m_thrust_strength = 0.0f;
  float m_sensitivity = 0.f;
  float m_yaw_rad = 0.f;
  float m_pitch_rad = 0.f;
  float m_fovy_rad = 0.f;

  bool m_dirty = true;
  glm::mat4 m_view = glm::mat4(1.f);
  glm::mat4 m_proj = glm::mat4(1.f);

  glm::vec3 get_camera_move_dir(App &app) const;

public:
  void setup(SceneObjectPtr player, float aspect_ratio, float thrust_strength = 0.5f,
             float sensitivity = 1.0f, float fovy_deg = 60.0f,
             float z_near = 0.1f, float z_far = 100.0f);

  glm::mat4 get_view_matrix();
  glm::mat4 get_projection_matrix() const;

  void update(App &app);
  void late_update();

  float get_fovy_rad() const;
  float get_sensitivity() const;
  void set_sensitivity(float sensitivity);

  friend class App;
};
