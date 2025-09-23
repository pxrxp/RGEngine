#include "TransformComponent.h"
#include <glm/gtc/matrix_transform.hpp>

TransformComponent::TransformComponent(const TransformConfig& config)
    : m_position(config.position),
      m_orientation(config.orientation),
      m_scale(config.scale),
      m_dirty(true),
      m_model_matrix(1.0f) {}

bool TransformComponent::is_dirty() const {
    return m_dirty;
}

TransformComponent& TransformComponent::set_dirty() {
    m_dirty = true;
    return *this;
}

TransformComponent& TransformComponent::set_position(const glm::vec3& position) {
    m_position = position;
    m_dirty = true;
    return *this;
}

TransformComponent& TransformComponent::set_rotation(const glm::quat& orientation) {
    m_orientation = glm::normalize(orientation);
    m_dirty = true;
    return *this;
}

TransformComponent& TransformComponent::set_scale(const glm::vec3& scale) {
    m_scale = scale;
    m_dirty = true;
    return *this;
}

TransformComponent& TransformComponent::translate(const glm::vec3& offset) {
    m_position += offset;
    m_dirty = true;
    return *this;
}

TransformComponent& TransformComponent::rotate(const glm::vec3& axis, float angle_rad) {
    m_orientation = glm::normalize(glm::angleAxis(angle_rad, axis) * m_orientation);
    m_dirty = true;
    return *this;
}

TransformComponent& TransformComponent::scale(const glm::vec3& factors) {
    m_scale *= factors;
    m_dirty = true;
    return *this;
}

const glm::vec3& TransformComponent::get_position() const { return m_position; }
const glm::quat& TransformComponent::get_rotation() const { return m_orientation; }
const glm::vec3& TransformComponent::get_scale() const { return m_scale; }

const glm::mat4& TransformComponent::get_model_matrix() const {
    if (m_dirty) {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), m_position);
        glm::mat4 R = glm::mat4_cast(m_orientation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), m_scale);

        m_model_matrix = T * R * S;
        m_dirty = false;
    }
    return m_model_matrix;
}
