#include <engine/Camera.h>
#include <engine/App.h>
#include <engine/SceneObject.h>
#include <cmath>

#ifndef NDEBUG
#include <engine/debug.h>
#endif

Camera&
Camera::setup(SceneObjectStrongPtr player)
{
  m_target_player = player;
  return *this;
}

glm::vec3
Camera::get_forward()
{
  if (!m_target_player)
    return glm::vec3(0.0f, 0.0f, -1.0f);
  return m_target_player->get_local_transform().get_rotation() *
         glm::vec3(0.0f, 0.0f, -1.0f);
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
    m_euler_angles.x += -delta_mouse.y * px_to_rad * m_sensitivity;

    const float pitch_limit = glm::pi<float>() / 2.0f - 0.01f;
    m_euler_angles.x = glm::clamp(m_euler_angles.x, -pitch_limit, pitch_limit);
    m_view_dirty = true;
  }

  glm::quat player_orientation =
    glm::quat({ m_euler_angles.x, m_euler_angles.y, 0.0f });
  m_target_player->get_local_transform().set_rotation(player_orientation);
}

void
Camera::update_projection_matrix(float aspect_ratio)
{
  m_aspect_ratio_cache = aspect_ratio;
  m_proj =
    glm::perspective(m_fovy_rad, m_aspect_ratio_cache, m_z_near, m_z_far);
  m_proj_dirty = false;
}

Camera&
Camera::look_at(const glm::vec3& target)
{
  if (!m_target_player)
    return *this;

  const glm::vec3 position = m_target_player->get_local_transform().get_position();
  glm::vec3 direction = target - position;
  const float direction_length_sq = glm::dot(direction, direction);

  // glm::normalize(vec3(0)) is undefined and produces NaNs. Keep the current
  // orientation if asked to look at the camera's own position.
  if (!std::isfinite(direction_length_sq) || direction_length_sq < 1e-8f) {
    reset_mouse_cache();
    m_view_dirty = true;
    return *this;
  }

  direction /= std::sqrt(direction_length_sq);
  // Avoid a degenerate up/direction pair when looking almost vertically.
  const glm::vec3 up = std::abs(glm::dot(direction, glm::vec3(AXIS_Y))) > 0.999f
                         ? glm::vec3(0.0f, 0.0f, 1.0f)
                         : glm::vec3(AXIS_Y);
  m_target_player->get_local_transform().set_rotation(glm::quatLookAt(direction, up));
  reset_mouse_cache();
  m_view_dirty = true;
  return *this;
}

void
Camera::reset_mouse_cache()
{
  m_euler_angles =
    glm::eulerAngles(m_target_player->get_local_transform().get_rotation());
}

void
Camera::update_view_matrix()
{
  if (!m_target_player)
    return;

  glm::vec3 player_pos = m_target_player->get_local_transform().get_position();
  glm::quat player_orientation =
    m_target_player->get_local_transform().get_rotation();
  glm::vec3 forward = player_orientation * glm::vec3(AXIS_NEG_Z);

  m_view = glm::lookAt(player_pos, player_pos + forward, glm::vec3(AXIS_Y));
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

glm::mat4
Camera::get_projection_matrix()
{
  if (m_proj_dirty) {
    update_projection_matrix(m_aspect_ratio_cache);
  }
  return m_proj;
}

Camera&
Camera::set_fovy(float fovy_deg)
{
  m_fovy_rad = glm::radians(fovy_deg);
  m_proj_dirty = true;
  return *this;
}
Camera&
Camera::set_fovy_rad(float fovy_rad)
{
  m_fovy_rad = fovy_rad;
  m_proj_dirty = true;
  return *this;
}

Camera&
Camera::set_sensitivity(float sensitivity)
{
  m_sensitivity = glm::max(0.f, sensitivity);
  return *this;
}

Camera&
Camera::set_speed(float speed)
{
  m_speed = glm::max(0.f, speed);
  return *this;
}

Camera&
Camera::set_far_plane(float z_far)
{
  m_z_far = glm::max(0.f, z_far);
  m_proj_dirty = true;
  return *this;
}

Camera&
Camera::set_near_plane(float z_near)
{
  m_z_near = glm::max(0.f, z_near);
  m_proj_dirty = true;
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
     << "  projection_matrix=" << cam.m_proj << "\n"
     << ")";
  return os;
}
#endif
