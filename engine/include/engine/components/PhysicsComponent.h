#pragma once
#include <engine/components/TransformComponent.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

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
  PhysicsComponent& apply_impulse_at_point(const glm::vec3& impulse,
                                           const glm::vec3& apply_point);
  PhysicsComponent& integrate_position();
  PhysicsComponent& integrate_velocity();

  float get_mass() const { return m_mass; }

  const glm::vec3 get_linear_momentum() const
  {
    return m_linear_momentum_half_step;
  }
  const glm::vec3 get_angular_momentum() const
  {
    return m_angular_momentum_half_step;
  }
  PhysicsComponent& set_linear_momentum(const glm::vec3& momentum)
  {
    m_linear_momentum_half_step = momentum;
    return *this;
  }
  PhysicsComponent& set_angular_momentum(const glm::vec3& momentum)
  {
    m_angular_momentum_half_step = momentum;
    return *this;
  }

  bool has_gravity() const { return m_has_gravity; }

  void enable() { m_enabled = true; }
  void disable() { m_enabled = false; }
  bool is_enabled() const { return m_enabled; }

  PhysicsComponent& set_mass(float mass);
  PhysicsComponent& set_gravity(bool has_gravity);

#ifndef NDEBUG
  friend std::ostream& operator<<(std::ostream& os,
                                  const PhysicsComponent& obj);
#endif

private:
  const glm::vec3 get_angular_velocity() const;

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
  bool m_enabled = true;
};
