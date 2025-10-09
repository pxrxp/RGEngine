#include "PhysicsComponent.h"
#include <glm/gtc/quaternion.hpp>

#ifndef NDEBUG
#include <iomanip>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#endif

const glm::vec3
PhysicsComponent::get_velocity() const
{
  return m_inverse_mass * m_linear_momentum_half_step;
}

const glm::vec3
PhysicsComponent::get_angular_velocity() const
{
  auto rotation_matrix = glm::toMat3(m_local_transform.get_rotation());
  auto world_inverse_inertia_tensor = rotation_matrix *
                                      m_inverse_inertia_tensor *
                                      glm::transpose(rotation_matrix);
  return world_inverse_inertia_tensor * m_angular_momentum_half_step;
}

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
PhysicsComponent::set_drag(float drag)
{
  m_drag = drag;
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
PhysicsComponent::set_initial_conditions(
  const PhysicsInitialConditions& initial)
{
  if (!m_initial_conditions_set) {
    if (std::isfinite(m_mass)) {
      m_linear_momentum_half_step = m_mass * initial.velocity;
    } else {
      m_linear_momentum_half_step = glm::vec3(0.f);
    }

    if (glm::length2(initial.angular_velocity) > 0.0001f) {
      auto rotationMatrix = glm::toMat3(m_local_transform.get_rotation());
      auto world_inertia_tensor = rotationMatrix *
                                  glm::inverse(m_inverse_inertia_tensor) *
                                  glm::transpose(rotationMatrix);
      m_angular_momentum_half_step =
        world_inertia_tensor * initial.angular_velocity;
    }

    m_initial_conditions_set = true;
  }
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
  m_linear_momentum_half_step += impulse;
  return *this;
}

PhysicsComponent&
PhysicsComponent::integrate_position()
{
  if (std::isfinite(m_linear_momentum_half_step.x) &&
      std::isfinite(m_linear_momentum_half_step.y) &&
      std::isfinite(m_linear_momentum_half_step.z))
    m_local_transform.translate(m_inverse_mass * m_linear_momentum_half_step *
                                m_fixed_step);

  auto delta_rotation_vector = get_angular_velocity() * m_fixed_step;

  float angle = glm::length2(delta_rotation_vector);
  if (angle > 0.0001f) {
    glm::vec3 axis = delta_rotation_vector / angle;
    m_local_transform.rotate(axis, angle);
  }

  return *this;
}

PhysicsComponent&
PhysicsComponent::integrate_velocity()
{
  m_linear_momentum_half_step += m_total_force * m_fixed_step;
  m_angular_momentum_half_step += m_total_torque * m_fixed_step;

  return *this;
}

#ifndef NDEBUG
std::ostream&
operator<<(std::ostream& os, const PhysicsComponent& obj)
{
  os << std::fixed << std::setprecision(2);

  os << "--- Physics Object State ---\n"
     << "Linear Momentum:       " << obj.m_linear_momentum_half_step << "\n"
     << "Angular Momentum:      " << obj.m_angular_momentum_half_step << "\n"
     << "----\n"
     << "Total Force:           " << obj.m_total_force << "\n"
     << "Total Torque:          " << obj.m_total_torque << "\n"
     << "----\n"
     << "Mass:                  " << obj.m_mass << "\n"
     << "Inverse Mass:          " << obj.m_inverse_mass << "\n"
     << "Inv. Inertia Tensor:   " << obj.m_inverse_inertia_tensor << "\n"
     << "----\n"
     << "Restitution:           " << obj.m_restitution << "\n"
     << "Drag:                  " << obj.m_drag << "\n"
     << "----\n"
     << std::boolalpha << "Enabled:               " << obj.m_enabled << "\n"
     << "Has Gravity:           " << obj.m_has_gravity << "\n"
     << "Initial Cond. Set:     " << obj.m_initial_conditions_set << "\n"
     << "Is First Step:         " << obj.m_is_first_step << "\n"
     << "---------------------------\n";
  return os;
}
#endif
