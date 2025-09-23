#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <initializer_list>

enum Where { Outside, Inside, AtBoundary };

struct Boundary {
  glm::vec3 min, max;

  Where where(const glm::vec3 &point) {
    bool at_x_boundary = point.x == min.x || point.x == max.x;
    bool at_y_boundary = point.y == min.y || point.y == max.y;
    bool at_z_boundary = point.z == min.z || point.z == max.z;

    bool at_boundary = at_x_boundary || at_y_boundary || at_z_boundary;

    if (at_boundary)
      return AtBoundary;

    bool inside_x_boundary = point.x > min.x && point.x < max.x;
    bool inside_y_boundary = point.y > min.y && point.y < max.y;
    bool inside_z_boundary = point.z > min.z && point.z < max.z;

    bool inside = inside_x_boundary && inside_y_boundary && inside_z_boundary;

    if (inside)
      return Inside;

    return Outside;
  }

  bool collides(const Boundary &other) {
    for (int x : {other.min.x, other.max.x}) {
      for (int y : {other.min.y, other.max.y}) {
        for (int z : {other.min.z, other.max.z}) {
          if (where(glm::vec3(x, y, z)) == Inside)
            return true;
        }
      }
    }
    return false;
  }
};

struct TransformConfig {
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
};

class TransformComponent {
public:
  explicit TransformComponent(const TransformConfig &config = {})
      : m_position(config.position), m_orientation(config.orientation),
        m_scale(config.scale), m_dirty(true) {}

  void set_dirty() { m_dirty = true; }

  void set_position(const glm::vec3 &position) {
    m_position = position;
    m_dirty = true;
  }

  void set_rotation(const glm::quat &orientation) {
    m_orientation = glm::normalize(orientation);
    m_dirty = true;
  }

  void set_scale(const glm::vec3 &scale) {
    m_scale = scale;
    m_dirty = true;
  }

  void translate(const glm::vec3 &offset) {
    m_position += offset;
    m_dirty = true;
  }

  void rotate(const glm::vec3 &axis, float angle_rad) {
    m_orientation =
        glm::normalize(glm::angleAxis(angle_rad, axis) * m_orientation);
    m_dirty = true;
  }

  void scale(const glm::vec3 &factors) {
    m_scale *= factors;
    m_dirty = true;
  }

  const glm::vec3 &get_position() const { return m_position; }
  const glm::quat &get_rotation() const { return m_orientation; }
  const glm::vec3 &get_scale() const { return m_scale; }

  const glm::mat4 &get_model_matrix() const {
    if (m_dirty) {
      glm::mat4 T = glm::translate(glm::mat4(1.0f), m_position);
      glm::mat4 R = glm::mat4_cast(m_orientation);
      glm::mat4 S = glm::scale(glm::mat4(1.0f), m_scale);

      m_model_matrix = T * R * S;
      m_dirty = false;
    }
    return m_model_matrix;
  }

private:
  glm::vec3 m_position;
  glm::quat m_orientation;
  glm::vec3 m_scale;

  mutable bool m_dirty;
  mutable glm::mat4 m_model_matrix;
};

struct PhysicsConfig {
  float mass = 1.0f;
  glm::mat3 inverse_inertia_tensor = glm::mat3(1.0f);
  bool has_gravity = true;
};

class PhysicsComponent {
public:
  explicit PhysicsComponent(const PhysicsConfig &config = {})
      : m_mass(config.mass > 0.0f ? config.mass : 0.0f),
        m_inverse_mass(config.mass > 0.0f ? 1.0f / config.mass : 0.0f),
        m_inverse_inertia_tensor(config.inverse_inertia_tensor),
        m_has_gravity(config.has_gravity) {}

  void clear_forces() {
    m_total_force = glm::vec3(0.0f);
    m_total_torque = glm::vec3(0.0f);
  }

  void apply_force(const glm::vec3 &force) { m_total_force += force; }

  void apply_force_at_point(const glm::vec3 &force,
                            const glm::vec3 &center_of_mass,
                            const glm::vec3 &apply_point) {
    m_total_force += force;
    glm::vec3 r = apply_point - center_of_mass;
    m_total_torque += glm::cross(r, force);
  }

  void apply_linear_impulse(const glm::vec3 &impulse) {
    m_linear_momentum += impulse;
  }

  void integrate(float fixed_step) {
    m_linear_momentum += m_total_force * fixed_step;
    m_angular_momentum += m_total_torque * fixed_step;
    m_velocity = m_linear_momentum * m_inverse_mass;
    m_angular_velocity = m_inverse_inertia_tensor * m_angular_momentum;
  }

  const glm::vec3 &get_velocity() const { return m_velocity; }
  const glm::vec3 &get_angular_velocity() const { return m_angular_velocity; }
  float get_mass() const { return m_mass; }
  bool has_gravity() const { return m_has_gravity; }

private:
  glm::vec3 m_velocity = glm::vec3(0.0f);
  glm::vec3 m_linear_momentum = glm::vec3(0.0f);
  glm::vec3 m_total_force = glm::vec3(0.0f);

  glm::vec3 m_angular_velocity = glm::vec3(0.0f);
  glm::vec3 m_angular_momentum = glm::vec3(0.0f);
  glm::vec3 m_total_torque = glm::vec3(0.0f);

  float m_mass;
  float m_inverse_mass;
  glm::mat3 m_inverse_inertia_tensor;
  bool m_has_gravity;
};
