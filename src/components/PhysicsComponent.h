#pragma once
#include <glm/glm.hpp>

struct PhysicsConfig {
    float mass = 1.0f;
    glm::mat3 inverse_inertia_tensor = glm::mat3(1.0f);
    bool has_gravity = true;
};

class PhysicsComponent {
public:
    PhysicsComponent(const PhysicsConfig& config = {});

    PhysicsComponent& clear_forces();
    PhysicsComponent& apply_force(const glm::vec3& force);
    PhysicsComponent& apply_force_at_point(const glm::vec3& force, const glm::vec3& center_of_mass, const glm::vec3& apply_point);
    PhysicsComponent& apply_linear_impulse(const glm::vec3& impulse);
    PhysicsComponent& integrate(float fixed_step);

    const glm::vec3& get_velocity() const;
    const glm::vec3& get_angular_velocity() const;
    float get_mass() const;
    bool has_gravity() const;

private:
    glm::vec3 m_velocity;
    glm::vec3 m_linear_momentum;
    glm::vec3 m_total_force;
    glm::vec3 m_angular_velocity;
    glm::vec3 m_angular_momentum;
    glm::vec3 m_total_torque;
    float m_mass;
    float m_inverse_mass;
    glm::mat3 m_inverse_inertia_tensor;
    bool m_has_gravity;
};
