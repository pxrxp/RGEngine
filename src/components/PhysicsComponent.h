#pragma once
#include <glm/glm.hpp>

class PhysicsComponent {
public:
  PhysicsComponent &clear_forces();
  PhysicsComponent &apply_force(const glm::vec3 &force);
  PhysicsComponent &apply_force_at_point(const glm::vec3 &force,
                                         const glm::vec3 &center_of_mass,
                                         const glm::vec3 &apply_point);
  PhysicsComponent &apply_linear_impulse(const glm::vec3 &impulse);
  PhysicsComponent &integrate(float fixed_step);

  const glm::vec3 &get_velocity() const;
  const glm::vec3 &get_angular_velocity() const;
  float get_mass() const;
  bool has_gravity() const;

  PhysicsComponent &set_mass(float mass);
  PhysicsComponent &set_gravity(bool has_gravity);
  PhysicsComponent &set_inverse_inertia_tensor(const glm::mat3 &tensor);

private:
  glm::vec3 m_velocity = glm::vec3(0.f);
  glm::vec3 m_linear_momentum = glm::vec3(0.f);
  glm::vec3 m_total_force = glm::vec3(0.f);
  glm::vec3 m_angular_velocity = glm::vec3(0.f);
  glm::vec3 m_angular_momentum = glm::vec3(0.f);
  glm::vec3 m_total_torque = glm::vec3(0.f);
  float m_mass = 1.f;
  float m_inverse_mass = 1.f;
  glm::mat3 m_inverse_inertia_tensor = glm::mat3(1.f);
  bool m_has_gravity = true;
};
