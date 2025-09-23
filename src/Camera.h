#pragma once

#include "Renderer.h"
#include "components.h"
#include <glm/glm.hpp>
#include <vector>

class App;

class Camera {
public:
  Camera& setup(SceneObjectPtr player);
  void update_lazy(App& app);
  void update_projection_matrix(float aspect_ratio);

  glm::mat4 get_view_matrix();
  glm::mat4 get_projection_matrix();
  BoundingBox get_bounding_box();

  Camera& look_at(const glm::vec3& target);
  glm::vec3 get_right();
  glm::vec3 get_up();
  glm::vec3 get_forward();

  float get_fovy_rad() const;
  Camera& set_fovy(float fovy_deg);

  float get_sensitivity() const;
  Camera& set_sensitivity(float sensitivity);

  float get_speed() const;
  Camera& set_speed(float speed);

  float get_far_plane() const;
  Camera& set_far_plane(float z_far);

  float get_near_plane() const;
  Camera& set_near_plane(float z_near);

private:
  glm::vec3 get_camera_move_dir(App& app) const;
  void update_view_matrix();

  SceneObjectPtr m_target_player;

  float m_speed = 5.f;
  float m_sensitivity = 0.5f;

  float m_yaw_rad = 0.f;
  float m_pitch_rad = 0.f;

  float m_fovy_rad = glm::radians(60.0f);
  float m_z_near = 0.1f;
  float m_z_far = 100.f;
  float m_aspect_ratio_cache = 16.f / 9.f;

  bool m_view_dirty = true;
  bool m_proj_dirty = true;
  glm::mat4 m_view = glm::mat4(1.f);
  glm::mat4 m_proj = glm::mat4(1.f);

  std::vector<glm::vec3> m_ortho_frustum;
  BoundingBox m_bounding_box;
};
