#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct TransformConfig {
    glm::vec3 position = glm::vec3(0.0f);
    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

class TransformComponent {
public:
    TransformComponent(const TransformConfig& config = {});

    bool is_dirty() const;
    TransformComponent& set_dirty();

    TransformComponent& set_position(const glm::vec3& position);
    TransformComponent& set_rotation(const glm::quat& orientation);
    TransformComponent& set_scale(const glm::vec3& scale);

    TransformComponent& translate(const glm::vec3& offset);
    TransformComponent& rotate(const glm::vec3& axis, float angle_rad);
    TransformComponent& scale(const glm::vec3& factors);

    const glm::vec3& get_position() const;
    const glm::quat& get_rotation() const;
    const glm::vec3& get_scale() const;
    const glm::mat4& get_model_matrix() const;

private:
    glm::vec3 m_position;
    glm::quat m_orientation;
    glm::vec3 m_scale;

    mutable bool m_dirty;
    mutable glm::mat4 m_model_matrix;
};
