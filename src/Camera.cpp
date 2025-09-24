#include "Camera.h"
#include "App.h"
#include "Renderer.h"
#include "Scene.h"
#include "components/BoundingBox.h"
#include "components/vertex_formats.h"
#include "definitions.h"
#include "opengl.h"
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#include "debug.h"

glm::vec3
Camera::get_camera_move_dir(App& app) const
{
  glm::vec3 direction(0.f);

  bool go_forward = app.get_window().is_key_pressed(GLFW_KEY_W) ||
                    app.get_window().is_key_pressed(GLFW_KEY_UP);
  bool go_backward = app.get_window().is_key_pressed(GLFW_KEY_S) ||
                     app.get_window().is_key_pressed(GLFW_KEY_DOWN);
  bool go_left = app.get_window().is_key_pressed(GLFW_KEY_A) ||
                 app.get_window().is_key_pressed(GLFW_KEY_LEFT);
  bool go_right = app.get_window().is_key_pressed(GLFW_KEY_D) ||
                  app.get_window().is_key_pressed(GLFW_KEY_RIGHT);
  bool go_up = app.get_window().is_key_pressed(GLFW_KEY_E);
  bool go_down = app.get_window().is_key_pressed(GLFW_KEY_Q);
  bool go_fast = app.get_window().is_key_pressed(GLFW_KEY_LEFT_CONTROL) ||
                 app.get_window().is_key_pressed(GLFW_KEY_RIGHT_CONTROL);
  bool go_slow = app.get_window().is_key_pressed(GLFW_KEY_LEFT_SHIFT) ||
                 app.get_window().is_key_pressed(GLFW_KEY_RIGHT_SHIFT);

  if (go_forward)
    direction.z += 1.0f;
  if (go_backward)
    direction.z -= 1.0f;
  if (go_left)
    direction.x -= 1.0f;
  if (go_right)
    direction.x += 1.0f;
  if (go_up)
    direction.y += 1.0f;
  if (go_down)
    direction.y -= 1.0f;

  if (glm::length(direction) > 0.0f) {
    direction = glm::normalize(direction);
  }

  if (go_fast)
    direction *= 2.0f;
  if (go_slow)
    direction /= 2.0f;

  return direction;
}

Camera&
Camera::setup(SceneObjectPtr player)
{
  m_target_player = player;
  return *this;
}

BoundingBox
Camera::get_AABB()
{
  if (m_view_dirty) {
    update_view_matrix();
  }
  return m_AABB;
}

void
Camera::update_lazy(App& app)
{
  if (!m_target_player)
    return;

  auto delta_mouse = app.get_window().get_delta_mouse();
  if (delta_mouse.x != 0 || delta_mouse.y != 0) {
    float px_to_rad = m_fovy_rad / app.get_window().get_height();
    m_euler_angles.y += -delta_mouse.x * px_to_rad * m_sensitivity;
    m_euler_angles.x += delta_mouse.y * px_to_rad * m_sensitivity;
    const float pitch_limit = glm::pi<float>() / 2.0f - 0.01f;
    m_euler_angles.x = glm::clamp(m_euler_angles.x, -pitch_limit, pitch_limit);
    m_view_dirty = true;
  }

  glm::quat player_orientation =
    glm::quat({ m_euler_angles.x, m_euler_angles.y, 0.0f });
  m_target_player->local_transform.set_rotation(player_orientation);

  auto move_dir = get_camera_move_dir(app);
  if (glm::length(move_dir) > 0.0f) {
    glm::vec3 forward = player_orientation * glm::vec3(AXIS_NEG_Z);
    glm::vec3 right = player_orientation * glm::vec3(AXIS_X);
    glm::vec3 up = glm::vec3(AXIS_Y);

    glm::vec3 move_force =
      (right * move_dir.x + up * move_dir.y + forward * move_dir.z) * m_speed;
    m_target_player->local_transform.translate(move_force *
                                               app.get_delta_time());
    m_view_dirty = true;
  }
}

void
Camera::update_projection_matrix(float aspect_ratio)
{
  m_aspect_ratio_cache = aspect_ratio;
  m_proj =
    glm::perspective(m_fovy_rad, m_aspect_ratio_cache, m_z_near, m_z_far);

  float top = m_z_far * tan(m_fovy_rad / 2);
  float right = top * m_aspect_ratio_cache;

  m_ortho_frustum = {
    { -right, -top, m_z_near }, { right, -top, m_z_near },
    { right, top, m_z_near },   { -right, top, m_z_near },
    { -right, -top, m_z_far },  { right, -top, m_z_far },
    { right, top, m_z_far },    { -right, top, m_z_far },
  };
}

Camera&
Camera::look_at(const glm::vec3& target)
{
  auto orientation = glm::quatLookAt(
    glm::normalize(target - m_target_player->local_transform.get_position()),
    AXIS_Y);
  m_euler_angles = glm::eulerAngles(orientation);
  m_view_dirty = true;
  return *this;
}

void
Camera::update_view_matrix()
{
  if (!m_target_player)
    return;

  glm::vec3 player_pos = m_target_player->local_transform.get_position();
  glm::quat player_orientation =
    m_target_player->local_transform.get_rotation();
  glm::vec3 forward = player_orientation * glm::vec3(AXIS_NEG_Z);

  m_view = glm::lookAt(player_pos, player_pos + forward, glm::vec3(AXIS_Y));

  std::vector<SimpleVertex> points_worldspace;
  for (auto& point : m_ortho_frustum) {
    points_worldspace.push_back(
      { glm::vec3(glm::inverse(m_view) * glm::vec4(point, 1.0f)) });
  }
  m_AABB = BoundingBox(points_worldspace);

  m_view_dirty = false;
}

glm::mat4
Camera::get_view_matrix()
{
  if (m_view_dirty) {
    update_view_matrix();
  }
  return m_view;
}

glm::vec3
Camera::get_right()
{
  glm::mat4 V = get_view_matrix();
  return glm::vec3(V[0]);
}

glm::vec3
Camera::get_up()
{
  glm::mat4 V = get_view_matrix();
  return glm::vec3(V[1]);
}

glm::vec3
Camera::get_forward()
{
  glm::mat4 V = get_view_matrix();
  return -glm::vec3(V[2]);
}

glm::mat4
Camera::get_projection_matrix()
{
  if (m_proj_dirty) {
    update_projection_matrix(m_aspect_ratio_cache);
  }
  return m_proj;
}

float
Camera::get_fovy_rad() const
{
  return m_fovy_rad;
}
Camera&
Camera::set_fovy(float fovy_deg)
{
  m_fovy_rad = glm::radians(fovy_deg);
  m_proj_dirty = true;
  return *this;
}

float
Camera::get_sensitivity() const
{
  return m_sensitivity;
}
Camera&
Camera::set_sensitivity(float sensitivity)
{
  m_sensitivity = glm::max(0.f, sensitivity);
  return *this;
}

float
Camera::get_speed() const
{
  return m_speed;
}
Camera&
Camera::set_speed(float speed)
{
  m_speed = glm::max(0.f, speed);
  return *this;
}

float
Camera::get_far_plane() const
{
  return m_z_far;
}
Camera&
Camera::set_far_plane(float z_far)
{
  m_z_far = glm::max(0.f, z_far);
  return *this;
}

float
Camera::get_near_plane() const
{
  return m_z_near;
}
Camera&
Camera::set_near_plane(float z_near)
{
  m_z_near = glm::max(0.f, z_near);
  return *this;
}

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const Camera& cam)
{
  os << "Camera(\n"
     << "  target_player=" << cam.m_target_player.get() << ",\n"
     << "  speed=" << cam.m_speed << ", sensitivity=" << cam.m_sensitivity
     << ",\n"
     << "  yaw_rad=" << cam.m_euler_angles.y
     << ", pitch_rad=" << cam.m_euler_angles.x << ",\n"
     << "  fovy_rad=" << cam.m_fovy_rad << ", near=" << cam.m_z_near
     << ", far=" << cam.m_z_far << ",\n"
     << "  aspect_ratio_cache=" << cam.m_aspect_ratio_cache << ",\n"
     << "  view_dirty=" << cam.m_view_dirty
     << ", proj_dirty=" << cam.m_proj_dirty << ",\n"
     << "  view_matrix=" << cam.m_view << ",\n"
     << "  projection_matrix=" << cam.m_proj << ",\n"
     << "  AABB=" << cam.m_AABB << "\n"
     << ")";
  return os;
}
#endif
