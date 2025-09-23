#pragma once
#include "Renderer.h"
#include "components.h"
#include <glm/glm.hpp>

class App;

class Camera {

  SceneObjectPtr m_target_player;

  float m_speed = 0.0f;
  float m_sensitivity = 0.f;
  float m_yaw_rad = 0.f;
  float m_pitch_rad = 0.f;
  float m_fovy_rad = 0.f;

  std::vector<glm::vec3> m_ortho_frustum;
  BoundingBox m_bounding_box;

  bool m_dirty = true;
  glm::mat4 m_view = glm::mat4(1.f);
  glm::mat4 m_proj = glm::mat4(1.f);

  glm::vec3 get_camera_move_dir(App &app) const;

public:
  void setup(SceneObjectPtr player, float aspect_ratio, float speed = 0.005f,
             float sensitivity = 1.0f, float fovy_deg = 60.0f,
             float z_near = 0.1f, float z_far = 100.0f);

  glm::mat4 get_view_matrix();
  glm::mat4 get_projection_matrix() const;

  void update(App &app);
  void late_update();

  BoundingBox get_bounding_box();

  float get_fovy_rad() const;
  float get_sensitivity() const;
  void set_sensitivity(float sensitivity);

  friend class App;
};
