#include "PhysicsComponent.h"

PhysicsComponent&
PhysicsComponent::set_mass(float mass)
{
  m_mass = mass > 0.0f ? mass : 0.0f;
  m_inverse_mass = mass > 0.0f ? 1.0f / mass : 0.0f;
  return *this;
}

PhysicsComponent&
PhysicsComponent::set_gravity(bool has_gravity)
{
  m_has_gravity = has_gravity;
  return *this;
}

PhysicsComponent&
PhysicsComponent::set_restitution(float restitution)
{
  m_restitution = restitution;
  return *this;
}

PhysicsComponent&
PhysicsComponent::set_inverse_inertia_tensor(const glm::mat3& tensor)
{
  m_inverse_inertia_tensor = tensor;
  return *this;
}

PhysicsComponent&
PhysicsComponent::clear_forces()
{
  m_total_force = glm::vec3(0.0f);
  m_total_torque = glm::vec3(0.0f);
  return *this;
}

PhysicsComponent&
PhysicsComponent::apply_force(const glm::vec3& force)
{
  m_total_force += force;
  return *this;
}

PhysicsComponent&
PhysicsComponent::apply_force_at_point(const glm::vec3& force,
                                       const glm::vec3& center_of_mass,
                                       const glm::vec3& apply_point)
{
  m_total_force += force;
  glm::vec3 r = apply_point - center_of_mass;
  m_total_torque += glm::cross(r, force);
  return *this;
}

PhysicsComponent&
PhysicsComponent::apply_linear_impulse(const glm::vec3& impulse)
{
  m_linear_momentum += impulse;
  return *this;
}

PhysicsComponent&
PhysicsComponent::integrate(float fixed_step)
{
  m_linear_momentum += m_total_force * fixed_step;
  m_angular_momentum += m_total_torque * fixed_step;
  m_velocity = m_linear_momentum * m_inverse_mass;
  m_angular_velocity = m_inverse_inertia_tensor * m_angular_momentum;
  return *this;
}

const glm::vec3&
PhysicsComponent::get_velocity() const
{
  return m_velocity;
}
const glm::vec3&
PhysicsComponent::get_angular_velocity() const
{
  return m_angular_velocity;
}
float
PhysicsComponent::get_mass() const
{
  return m_mass;
}
bool
PhysicsComponent::has_gravity() const
{
  return m_has_gravity;
}
float
PhysicsComponent::get_restitution() const
{
  return m_restitution;
}
