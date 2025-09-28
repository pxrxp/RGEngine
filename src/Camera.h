#pragma once

#include "Renderer.h"
#include "components/BoundingBox.h"
#include "components/vertex_formats.h"
#include <glm/glm.hpp>
#include <vector>

class App;

class Camera
{
public:
#ifndef NDEBUG
  friend std::ostream& operator<<(std::ostream& os, const Camera& cam);
#endif

  Camera& setup(SceneObjectStrongPtr player);
  void update_lazy(App& app);
  void update_projection_matrix(float aspect_ratio);

  glm::mat4 get_view_matrix();
  glm::mat4 get_projection_matrix();

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

  const std::vector<Vertex_Pos>& get_frustum_worldspace();
  const std::vector<Vertex_Pos>& get_frustum_viewspace();
  const BoundingBox& get_AABB();

private:
  glm::vec3 get_camera_move_dir(App& app) const;
  void update_view_matrix();

  SceneObjectStrongPtr m_target_player;

  float m_speed = 3.f;
  float m_sensitivity = 0.5f;

  glm::vec3 m_euler_angles = { 0.f, 0.f, 0.f };

  float m_fovy_rad = glm::radians(60.0f);
  float m_z_near = 0.1f;
  float m_z_far = 100.f;
  float m_aspect_ratio_cache = 16.f / 9.f;

  bool m_frustum_viewspace_updated = true;
  bool m_frustum_worldspace_updated = true;
  bool m_view_dirty = true;
  bool m_proj_dirty = true;
  glm::mat4 m_view = glm::mat4(1.f);
  glm::mat4 m_proj = glm::mat4(1.f);

  std::vector<Vertex_Pos> m_frustum_viewspace;
  std::vector<Vertex_Pos> m_frustum_worldspace;
  BoundingBox m_AABB;
};
