#pragma once
#include "TransformComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

struct PhysicsInitialConditions
{
  glm::vec3 velocity = glm::vec3(0.f);
  glm::vec3 angular_velocity = glm::vec3(0.f);
};

class PhysicsComponent
{
public:
  PhysicsComponent(TransformComponent& local_transform, float fixed_step)
    : m_local_transform(local_transform)
    , m_fixed_step(fixed_step)
  {
  }

  PhysicsComponent& clear_forces();
  PhysicsComponent& apply_force(const glm::vec3& force);
  PhysicsComponent& apply_force_at_point(const glm::vec3& force,
                                         const glm::vec3& center_of_mass,
                                         const glm::vec3& apply_point);
  PhysicsComponent& apply_linear_impulse(const glm::vec3& impulse);
  PhysicsComponent& integrate_position();
  PhysicsComponent& integrate_velocity();

  float get_mass() const { return m_mass; }
  float get_inverse_mass() const { return m_inverse_mass; }
  const glm::vec3 get_total_force() const { return m_total_force; }
  const glm::vec3 get_total_torque() const { return m_total_torque; }
  float get_drag() const { return m_drag; }
  float get_restitution() const { return m_restitution; }
  const glm::mat3 get_inverse_inertia_tensor() const
  {
    return m_inverse_inertia_tensor;
  }
  const glm::vec3 get_linear_momentum() const
  {
    return m_linear_momentum_half_step;
  }
  const glm::vec3 get_angular_momentum() const
  {
    return m_angular_momentum_half_step;
  }
  bool has_gravity() const { return m_has_gravity; }

  const glm::vec3 get_velocity() const;
  const glm::vec3 get_angular_velocity() const;

  void enable() { m_enabled = true; }
  void disable() { m_enabled = false; }
  bool is_enabled() { return m_enabled; }

  PhysicsComponent& set_mass(float mass);
  PhysicsComponent& set_drag(float drag);
  PhysicsComponent& set_restitution(float restitution);
  PhysicsComponent& set_gravity(bool has_gravity);
  PhysicsComponent& set_inverse_inertia_tensor(const glm::mat3& tensor);

  PhysicsComponent& set_initial_conditions(
    const PhysicsInitialConditions& initial);

#ifndef NDEBUG
  friend std::ostream& operator<<(std::ostream& os,
                                  const PhysicsComponent& obj);
#endif

private:
  TransformComponent& m_local_transform;
  float m_fixed_step = 1.f;

  glm::vec3 m_linear_momentum_half_step = glm::vec3(0.f);
  glm::vec3 m_angular_momentum_half_step = glm::vec3(0.f);

  glm::vec3 m_total_force = glm::vec3(0.f);
  glm::vec3 m_total_torque = glm::vec3(0.f);

  float m_mass = 1.f;
  float m_inverse_mass = 1.f;
  glm::mat3 m_inverse_inertia_tensor = glm::mat3(1.f);

  bool m_has_gravity = true;
  float m_restitution = 0.5f;
  float m_drag = 0.5f;
  bool m_enabled = true;
  bool m_initial_conditions_set = false;
  bool m_is_first_step = true;
};
